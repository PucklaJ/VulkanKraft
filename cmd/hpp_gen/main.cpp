#include <filesystem>
#include <fstream>
#include <vector>

int main(int args, char *argv[]) {
  // Arguments
  const std::filesystem::path sourcefile(argv[1]), output_directory(argv[2]);

  std::filesystem::path destination_file(output_directory /
                                         sourcefile.filename());
  destination_file.replace_extension("hpp");
  std::filesystem::create_directories(destination_file.parent_path());

  std::ofstream dst_file;
  std::ifstream src_file;

  dst_file.open(destination_file);
  if (dst_file.fail()) {
    return 1;
  }
  src_file.open(sourcefile, std::ios_base::binary | std::ios_base::ate);
  if (src_file.fail()) {
    return 2;
  }

  dst_file << "#pragma once" << std::endl;
  dst_file << "#include <array>" << std::endl;
  dst_file << "#include <cstdint>" << std::endl;

  const auto src_file_size{src_file.tellg()};
  std::vector<uint8_t> src_file_data(src_file_size);

  dst_file << "constexpr auto " << destination_file.stem().string() << "_"
           << sourcefile.extension().string().substr(1)
           << " = std::array<uint8_t, " << src_file_size << ">{";

  src_file.seekg(0);
  src_file.read(reinterpret_cast<char *>(src_file_data.data()), src_file_size);

  for (const auto &data : src_file_data) {
    dst_file << static_cast<uint32_t>(data) << ", ";
  }
  dst_file << "};" << std::endl;

  return 0;
}
