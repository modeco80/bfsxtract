// bfsxtract
// Program for extracting
// BFS files from "Find My Own Way".
//
// (c) 2022 Lily/modeco80 <lily.modeco80@protonmail.ch>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string_view>

#include <elfio/elfio.hpp>

namespace eio = ELFIO;
namespace fs = std::filesystem;

namespace fishes {

	// Type for repressenting ps2 32-bit pointers.
	using ps2_ptr_t = std::uint32_t;

	enum class bfsID : std::uint32_t {
		// Null file. Don't use
		Invalid,
		Hdreg1_Vu1,
		Hdreg2_Vu1,
		Hdregu_Rmi,
		K_Vrtb_Rmi,
		Nhead_Rmi,
		P_Rtx,
		Tex_Rtx,
		Wat_Rtx,
		T_Smooth_Rmi
	};

	static_assert(bfsID::T_Smooth_Rmi == static_cast<bfsID>(9), "Invalid bfs ID table");

	std::string BfsIdToFileName(bfsID id) {
		constexpr const char* table[] = {
			"<null_file>", // don't write this to disk!!!

			// Actual filenames
			"hdreg1.vu1",
			"hdreg2.vu1",
			"hdregu.rmi",
			"k_vrtb.rmi",
			"nhead.rmi",
			"p.rtx",
			"tex.rtx",
			"wat.rtx",
			"t_smooth.rmi"
		};

		return table[static_cast<uint32_t>(id)];
	}

	/**
     * BFS file table entry.
     */
	struct bfsTabEntry {
		/**
		 * The start address.
		 */
		ps2_ptr_t startAddress;

		/**
		 * Address of the _END symbol.
		 */
		ps2_ptr_t endAddress;

		// Incremented by game. Don't know what use it has.
		std::uint32_t useCount;

		// not used.
		std::uint32_t pad_to_10h;

		/**
         * Get the length of this file.
         */
		[[nodiscard]] constexpr std::size_t Length() const {
			return static_cast<std::size_t>(endAddress - startAddress);
		}
	};

	static_assert(sizeof(bfsTabEntry) == 0x10, "Broken bfsTabEntry");
} // namespace fishes

/**
 * BFS reader.
 */
struct BfsReader {
	explicit BfsReader(const std::string& filename) {
		if(elfFile.load(filename))
			valid = true;

		if(valid) {
			dataSec = elfFile.sections[".data"];
			symAccessor = std::make_shared<eio::symbol_section_accessor>(elfFile, elfFile.sections[".symtab"]);

			eio::Elf64_Addr bfsTabStartAddress;

			{
				// "filler" variables (we don't use these since the data is always the same size, or we don't
				// care about the contents of some values)
				eio::Elf_Xword bfsTabSize;
				unsigned char fill;
				ELFIO::Elf_Half fillhalf;

				symAccessor->get_symbol("bfsTab", bfsTabStartAddress, bfsTabSize, fill, fill, fillhalf, fill);
			}

			// Read/cache bfsTab from the ELF file.
			for(int i = 0; i < 10; ++i) {
				auto& tableIndex = reinterpret_cast<const fishes::bfsTabEntry*>(dataSec->get_data() + UnsectionedAddress(bfsTabStartAddress, dataSec))[i];
				//std::printf("file %d: %08x %08x\n", i, tableIndex.startAddress, tableIndex.endAddress);
				cachedTable[i] = tableIndex;
			}
		}
	}

	std::vector<std::uint8_t> ReadFile(std::uint32_t id) {
		// refuse to extract the "null" file.
		if(static_cast<fishes::bfsID>(id) == fishes::bfsID::Invalid)
			return {};

		if(!valid)
			return {};

		std::vector<std::uint8_t> buffer;

		auto& tEntry = cachedTable[id];
		auto* fileStart = (dataSec->get_data() + UnsectionedAddress(tEntry.startAddress, dataSec));
		buffer.resize(tEntry.Length());

		memcpy(&buffer[0], fileStart, tEntry.Length());
		return buffer;
	}

	[[nodiscard]] bool Valid() const {
		return valid;
	}

	// TODO: better name for this
	static std::size_t UnsectionedAddress(std::size_t addr, eio::section* section) {
		return (addr - section->get_address());
	}

   private:
	bool valid { false };

	eio::elfio elfFile;
	eio::section* dataSec {};
	std::shared_ptr<eio::symbol_section_accessor> symAccessor;

	fishes::bfsTabEntry cachedTable[10] {};
};

int main() {
	BfsReader reader("FISHES.ELF");

	if(!reader.Valid()) {
		std::cout << "The provided ELF is invalid.\n";
		return 1;
	}

	auto outPath = fs::path("out");

	if(!fs::exists(outPath))
		fs::create_directories(outPath);

	for(std::uint32_t i = 1; i < 10; ++i) {
		auto fileOut = outPath / fishes::BfsIdToFileName(static_cast<fishes::bfsID>(i));
		auto buffer = reader.ReadFile(i);

		std::ofstream ofs(fileOut.string(), std::ofstream::binary);

		// shouldn't happen
		if(!ofs) {
			std::cout << "Couldn't open \"" << fileOut.string() << "\"?\n";
			continue;
		}

		ofs.write(reinterpret_cast<const char*>(&buffer[0]), buffer.size());

		std::cout << "Wrote \"" << fileOut.string() << "\"\n";
	}

	return 0;
}