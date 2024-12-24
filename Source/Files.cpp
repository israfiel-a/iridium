/**
 * @file Files.cpp
 * @author Israfil Argos (israfiel-a)
 * @date 2024-12-21
 * @brief The implementation file of Iridium's file management interface.
 *
 * Copyright (c) 2024 Israfil Argos
 * This file is under the AGPLv3. For more information on what that
 * entails, see the LICENSE file provided with the engine.
 */

#include <Files.hpp>
#include <fstream>
#include <iterator>

/**
 * @brief A fixed-width type that, when passed to an std::ostream, will
 * allow padding input.
 * @tparam T The type of input.
 * @tparam W The width of padding.
 */
template <typename T, int W>
struct FixedWidthValue
{
        FixedWidthValue(T v_) : v(v_) {}
        T v;
};

/**
 * @brief The default fixed-width stream operator. This does not set a
 * delimiter to fill with.
 * @tparam T The type of input.
 * @tparam W The width of padding.
 * @param output The output stream.
 * @param value The value we're printing.
 * @return The newly streamed-to output stream.
 */
template <typename T, int W>
std::ostream &operator<<(std::ostream &output,
                         const FixedWidthValue<T, W> &value)
{
    return output << std::setw(W) << value.v;
}

/**
 * @brief The fixed-width stream operator for unsigned characters. This
 * automatically prints a delimiter character if the byte can't be printed.
 * @tparam W The width of padding.
 * @param output The output stream.
 * @param value The value we're printing.
 * @return The newly streamed-to output stream.
 */
template <int W>
std::ostream &operator<<(std::ostream &output,
                         const FixedWidthValue<std::uint8_t, W> &value)
{
    // If the character is able to be printed, print it. Otherwise, print
    // the delimiter.
    if (isprint((int)value.v) && value.v != ' ')
        return output << value.v << " ";
    else return output << std::setw(W) << output.fill();
}

namespace Iridium::Files
{
    std::string MIME::GetFiletype(const FileContents &file_contents)
    {
        if (file_contents.size() > 7 && file_contents[0] == 0x89 &&
            file_contents[1] == 0x50 && file_contents[2] == 0x4E &&
            file_contents[3] == 0x47 && file_contents[4] == 0x0D &&
            file_contents[5] == 0x0A && file_contents[6] == 0x1A &&
            file_contents[7] == 0x0A)
            return "image/png";

        if (file_contents.size() > 3 && file_contents[0] == 0xFF &&
            file_contents[1] == 0xD8 && file_contents[2] == 0xFF &&
            (file_contents[3] == 0xDB || file_contents[3] == 0xE0))
            return "image/jpeg";

        if (file_contents.size() > 8 && file_contents[0] == 0x5D &&
            file_contents[1] == 0x49 && file_contents[2] == 0x72 &&
            file_contents[3] == 0x69 && file_contents[4] == 0x64 &&
            file_contents[5] == 0x69 && file_contents[6] == 0x75 &&
            file_contents[7] == 0x6D && file_contents[8] == 0x5D)
            return "text/config";

        return "application/octet-stream";
    }

    ImmutableFile::ImmutableFile(const FilePath &file_path)
    {
        path = NormalizePath(file_path);
        if (!std::filesystem::is_regular_file(path))
        {
            // No proper error recorder exists yet, so just fail the thread
            // as a placeholder.
            exit(255);
        }

        // Open the file in raw binary mode and with the position pointer
        // at the end of the buffer.
        std::ifstream file_stream(path, std::ios::binary | std::ios::ate);
        if (!file_stream.is_open())
        {
            // Follows the above comment.
            exit(255);
        }

        std::ifstream::pos_type file_size = file_stream.tellg();
        contents.resize(file_size);

        // Read any content.
        if (file_size > 0)
        {
            file_stream.seekg(0, std::ios::beg);
            file_stream.read((char *)contents.data(), file_size);
        }
    }

    const std::string &ImmutableFile::StringifyMetadata() const noexcept
    {
        static std::string metadata_string =
            "File \"" + GetBasename().string() + "\" (" +
            std::to_string(GetSize()) +
            " bytes):\n\tFull Path: " + GetPath().string() +
            "\n\tMIME Type: " + GetType() + "\n";
        return metadata_string;
    }

    void ImmutableFile::Hexdump(std::ostream &output,
                                std::size_t column_count,
                                bool characters) const
    {
        // Save the flags of the output stream so we can restore them
        // afterward.
        std::ios_base::fmtflags saved_flags(output.flags());
        output << std::uppercase << std::hex;

        const std::size_t max_rows = contents.size() / column_count;
        for (std::size_t current_row = 0; current_row < max_rows;
             ++current_row)
        {
            const std::size_t row_begin = current_row * column_count;
            const std::size_t row_end = row_begin + column_count;
            const std::vector<std::uint8_t> row(
                contents.begin() + row_begin, contents.begin() + row_end);

            output << std::setfill('0');
            // Print the line number.
            output << std::setw(8) << row_begin << "  ";
            // Copy the integer data into the given output stream.
            std::copy(row.begin(), row.end(),
                      std::ostream_iterator<FixedWidthValue<int, 2>>(
                          output, " "));

            if (characters)
            {
                output << "  " << std::setfill('.');
                std::copy(
                    row.begin(), row.end(),
                    std::ostream_iterator<
                        FixedWidthValue<std::uint8_t, 2>>(output, " "));
            }

            output << "\n";
        }

        // Reset flags and flush the stream.
        output.flags(saved_flags);
        output.flush();
    }

    FilePath NormalizePath(const FilePath &path)
    {
        std::filesystem::path canonical_path =
            std::filesystem::weakly_canonical(
                GetAssetDirectory().append(path.string()));
        return canonical_path.make_preferred();
    }

    FilePath GetAssetDirectory()
    {
        return std::filesystem::path("./Assets/");
    }
}
