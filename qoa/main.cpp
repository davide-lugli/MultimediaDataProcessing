#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>
#include <cmath>
#include <cstdlib>


void format() {
    std::cout << "QOA format error";
    exit(EXIT_FAILURE);
}

class little_bitwriter {
public:
    // Constructor: takes a reference to an output stream.
    little_bitwriter(std::ostream& os) : os_(os) {};

    // Destructor: should flush remaining bits.
    ~little_bitwriter() {
        flush();
    };

    void write_buf() {
        for (int8_t i = bytebuffer_.size() - 1; i >= 0; i--) {
            os_.put(bytebuffer_[i]);
        }
        bytebuffer_.clear();
    }

    // Writes the lower 'numbits' bits of 'x' to the stream.
    std::ostream& operator()(uint64_t x, uint64_t numbits) {
        while (numbits-- > 0) {
            writebit(x >> numbits);
        }
        write_buf();
        return os_;
    };

    // Flushes the current buffer, padding with 'padbit' (default 0) if needed.
    std::ostream& flush(int padbit = 0) {
        while (n_ > 0) {
            writebit(padbit);
        }
        write_buf();
        return os_;
    };

private:
    std::vector<uint8_t> bytebuffer_;
    // Internal buffer to collect bits.
    uint8_t buffer_;

    // Number of bits currently stored in the buffer.
    size_t n_ = 0;
    // Reference to the output stream.
    std::ostream& os_;

    // Helper function to write a single bit into the buffer.
    void writebit(uint64_t curbit) {
        buffer_ = (buffer_ << 1) | (curbit & 1);
        ++n_;
        if (n_ == 8) {
            bytebuffer_.emplace_back(buffer_);
            n_ = 0;
        }
    };
};

class bitreader {
public:
    // Constructor: takes a reference to an input stream.
    bitreader(std::istream& is) : is_(is) {};

    // Reads 'numbits' bits from the stream and returns them as a number.
    uint64_t operator()(uint64_t numbits) {
        uint64_t u = 0;
        while (numbits-- > 0) {
            u = (u << 1) | readbit();
        }
        return u;
    };

    // Returns true if the underlying stream is in a failed state.
    bool fail() const {
        return is_.fail();
    };

    // Allows using the object in boolean contexts (true if stream is OK).
    operator bool() const {
        return !fail();
    };

private:
    // Internal buffer to hold bits from the stream.
    uint8_t buffer_;
    // Number of bits left in the buffer.
    size_t n_ = 0;
    // Reference to the input stream.
    std::istream& is_;

    // Helper function to read a single bit from the buffer/stream.
    uint64_t readbit() {
        if (n_ == 0) {
            buffer_ = is_.get();
            n_ = 8;
        }
        --n_;
        return (buffer_ >> n_) & 1;
    };
};

struct qoa {
	struct file_header {
		std::string magic; // magic bytes "qoaf"
		std::uint32_t samples; // samples per channel in this file

        file_header() : magic(), samples(0) {
            magic.resize(4);
        }
	};
	
	struct frame { //[ceil(samples / (256 * 20))]{
		struct frame_header {
			std::uint8_t num_channels; // 2 FIXED no. of channels
			std::uint32_t samplerate; // 44100 FIXED samplerate in hz (24 bits)
			std::uint16_t fsamples; // samples per channel in this frame
			std::uint16_t fsize; // frame size (includes this header)
		};
		
		struct lms_state { //[num_channels] {
			std::vector<int16_t> history; // [4] most recent last
            std::vector<int16_t> weights; // [4] most recent last
		};

        struct slice {
            std::vector<float> dequant_tab = { 0.75, -0.75, 2.5, -2.5, 4.5, -4.5, 7, -7 };
            std::vector<int64_t> samples;

            slice(std::ifstream& is, lms_state& channel_info, uint32_t samples_left = 20) {
                bitreader br(is);

                // Read and dequantize scalefactor
                uint8_t sf_quant = br(4);
                uint64_t sf = std::round(std::pow(sf_quant + 1, 2.75));

                // Read 20 samples
                for (uint8_t i = 0; i < samples_left; i++) {
                    // Read and transform quantized residual
                    float qr = dequant_tab[br(3)];

                    // Calculate dequantized and scaled residual
                    double r_tmp = sf * qr;
                    int64_t r = (r_tmp < 0) ? std::ceil(r_tmp - 0.5) : std::floor(r_tmp + 0.5);

                    // Calculate predict sample
                    int64_t p = 0;
                    for (uint8_t j = 0; j < 4; j++) {
                        p += (channel_info.history[j] * channel_info.weights[j]);
                    }
                    p >>= 13;

                    // Calculate reconstructed sample
                    int64_t s = p + r;
                    if (s > 32767) {
                        s = 32767;
                    }
                    else if (s < -32768) {
                        s = -32768;
                    }

                    samples.emplace_back(s);
                    
                    // Update LMS weights & history
                    int64_t delta = r >> 4;
                    for (uint8_t j = 0; j < 4; j++) {
                        channel_info.weights[j] += (channel_info.history[j] < 0) ? -delta : delta;
                        channel_info.history[j] = (j == 3) ? s : channel_info.history[j + 1];
                    }
                }

                for (uint8_t i = 0; i < 20 - samples_left; i++) {
                    samples.emplace_back(0);
                }
            }
        };
		
        frame_header header;
        std::vector<lms_state> channel_info;
        std::vector<std::vector<slice>> slices; // [256][num_channels]

        frame(std::ifstream& is, uint32_t samples_left = 256 * 20) {
            bitreader br(is);

            // Read frame header
            header.num_channels = br(8);
            header.samplerate = br(24);
            header.fsamples = br(16);
            header.fsize = br(16);

            if (header.num_channels < 1 || header.samplerate < 1 || header.samplerate > 16777215) {
                format();
            }

            // Read channels LMS
            for (uint8_t i = 0; i < header.num_channels; i++) {
                lms_state ls;
                for (uint8_t j = 0; j < 4; j++) {
                    ls.history.emplace_back(br(16));
                }
                for (uint8_t j = 0; j < 4; j++) {
                    ls.weights.emplace_back(br(16));
                }
                channel_info.emplace_back(ls);

                slices.emplace_back();
            }

            // Read slices
            for (uint32_t i = 0; i < 256 * header.num_channels; i++) {
                size_t ci = i % header.num_channels;
                if (samples_left - 20 < 0) {
                    slices[ci].emplace_back(is, channel_info[ci], samples_left);
                    
                    if (ci == header.num_channels - 1) {
                        return;
                    }
                };
                slices[ci].emplace_back(is, channel_info[ci]);
                samples_left -= 20;
            }
        }
	};

    file_header header;
    std::vector<frame> frames;

    qoa(std::ifstream& is) {
        bitreader br(is);

        // Read file header
        is.read(&header.magic[0], 4);
        if (header.magic != "qoaf") {
            format();
        }
        header.samples = br(32);

        // Read frames
        uint32_t frames_number = std::ceil(header.samples / (256 * 20));
        for (uint32_t i = 0; i < frames_number; i++) {
            frames.emplace_back(is);
        }

        uint32_t samples_left = header.samples - (frames_number * 256 * 20);
        if (samples_left > 0) {
            frames.emplace_back(is, samples_left);
        }
    }
};

void write_wav(std::ofstream& os, qoa& qoa_file) {
    little_bitwriter bw(os);

    uint32_t datasize = qoa_file.header.samples * 2 * 2; // samples * channels * 2 byte
    std::cout << datasize;


    // Write header
    os << "RIFF";
    bw(datasize + 44 - 8, 32);
    os << "WAVE";
    os << "fmt ";
    bw(16, 32);
    bw(1, 16);
    bw(2, 16);
    bw(44100, 32);
    bw(176400, 32);
    bw(4, 16);
    bw(16, 16);
    os << "data";
    bw(datasize, 32);

    uint64_t ix = 0;
    for (const auto& frame : qoa_file.frames) {
        for (size_t i = 0; i < 256; i++) {
            for (size_t j = 0; j < 20; j++) {
                if (ix >= qoa_file.header.samples) {
                    std::cout << ix;
                    return;
                }
                bw(frame.slices[0][i].samples[j], 16);
                bw(frame.slices[1][i].samples[j], 16);
                ix++;
            }
        }
    }
}

int main(int argc, char* argv[]) {

	if (argc != 3) {
		std::cout << "SYNTAX: main <input_file> <output_file>";
		return EXIT_FAILURE;
	}

	std::ifstream is = std::ifstream(argv[1], std::ios::binary);
	if (!is) {
		std::cout << "Could not open input file";
		return EXIT_FAILURE;
	}

    qoa q(is);

    std::ofstream os = std::ofstream(argv[2], std::ios::binary);
    if (!os) {
        std::cout << "Could not open output file";
        return EXIT_FAILURE;
    }

    write_wav(os, q);

	return EXIT_SUCCESS;
}
