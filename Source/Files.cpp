#include <Files.hpp>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>

namespace Iridium
{
    template <typename T, std::uint32_t W>
    struct FixedWidthValue
    {
            FixedWidthValue(T v_) : v(v_) {}
            T v;
    };

    template <typename T, std::uint32_t W>
    std::ostream &operator<<(std::ostream &ostr,
                             const FixedWidthValue<T, W> &fwv)
    {
        return ostr << std::setw(W) << fwv.v;
    }

    template <std::uint32_t W>
    std::ostream &operator<<(std::ostream &ostr,
                             const FixedWidthValue<std::uint8_t, W> &fwv)
    {
        if (isprint((int)fwv.v) && fwv.v != ' ')
            return ostr << fwv.v << " ";
        else return ostr << std::setfill('.') << std::setw(W) << ".";
    }

    File::File(const std::string &path)
    {
        this->directory = GetAssetSubdirectoryFromPath(path);
        this->basename = GetBasenameFromPath(path);

        //! THIS FUNCTION ONLY WORKS IF CWD IS THE EXECUTABLE
        //! LOCATION!!!!!
        std::ifstream file_stream(GetApplicationAssetDirectory() + path,
                                  std::ios::binary | std::ios::ate);

        if (!file_stream.is_open())
        {
            //! Report error here.
            std::cout << GetApplicationAssetDirectory() + path << "\n"
                      << strerror(errno);
            exit(255);
        }

        //! rename this to size
        std::ifstream::pos_type pos = file_stream.tellg();
        if (pos == 0) { this->contents = std::vector<std::uint8_t>{}; }
        else
        {
            this->contents = std::vector<std::uint8_t>(pos);
            file_stream.seekg(0, std::ios::beg);
            file_stream.read((char *)this->contents.data(), pos);
        }

        file_stream.close();
    }

    const std::string &File::GetDirectory() const
    {
        return this->directory;
    }

    const std::string &File::GetBasename() const { return this->basename; }

    std::string File::GetType() const
    {
        static std::string mime = InferFiletype(this->contents);
        return mime;
    }

    std::ifstream::pos_type File::GetSize() const
    {
        // Not worth it to cache.
        return this->contents.size();
    }

    const std::vector<std::uint8_t> &File::GetContents() const
    {
        return this->contents;
    }

    std::string File::GetContentsAsString() const
    {
        static std::string stringified_contents(this->contents.begin(),
                                                this->contents.end());
        return stringified_contents;
    }

    void File::Hexdump(const std::ostream &output,
                       std::uint32_t column_count, bool metadata,
                       bool linenum, bool characters) const
    {
        if (metadata)
            std::cout << "\nFile '" << this->GetBasename() << "' ("
                      << this->GetSize() << " bytes):\n\tFull path: "
                      << GetApplicationAssetDirectory()
                      << this->GetDirectory() << this->GetBasename()
                      << "\n\tMIME type: " << this->GetType() << "\n\n";

        std::cout << std::uppercase << std::hex;
        for (std::uint32_t row_count = 0;
             row_count < this->contents.size() / column_count; row_count++)
        {
            std::cout << std::setfill('0');
            if (linenum)
                std::cout << std::setw(8) << row_count * column_count
                          << "  ";

            std::uint32_t index = row_count * column_count;
            const std::vector<std::uint8_t> row(
                this->contents.begin() + index,
                this->contents.begin() + index + column_count);

            std::copy(row.begin(), row.end(),
                      std::ostream_iterator<FixedWidthValue<int, 2>>(
                          std::cout, " "));

            if (characters)
            {
                std::cout << "  ";
                std::copy(
                    row.begin(), row.end(),
                    std::ostream_iterator<
                        FixedWidthValue<std::uint8_t, 2>>(std::cout, " "));
            }

            std::cout << "\n";
        }
        std::cout << std::dec;
    }

    std::string GetApplicationAssetDirectory()
    {
        // This will have further config in the future, so we have created
        // a boilerplate function to lay down the seeds.
        return "./Assets/";
    }

    std::string GetAssetSubdirectoryFromPath(const std::string &path)
    {
        const std::size_t last_directory_splitter = path.find_last_of('/');
        return path.substr(0, (last_directory_splitter != std::string::npos
                                   ? last_directory_splitter
                                   : 0));
    }

    std::string GetBasenameFromPath(const std::string &path)
    {
        // This will fail on Windows!
        const std::size_t last_directory_splitter = path.find_last_of('/');
        const std::size_t extension_dot = path.find_last_of('.');

        return path.substr(
            (last_directory_splitter != std::string::npos
                 ? last_directory_splitter
                 : 0),
            (extension_dot != std::string::npos ? extension_dot : 0));
    }

    std::string
    InferFiletype(const std::vector<std::uint8_t> &file_contents)
    {
        if (file_contents.at(0) == 0x89 && file_contents.at(1) == 0x50 &&
            file_contents.at(2) == 0x4E && file_contents.at(3) == 0x47 &&
            file_contents.at(4) == 0x0D && file_contents.at(5) == 0x0A &&
            file_contents.at(6) == 0x1A && file_contents.at(7) == 0x0A)
            return "image/png";

        if (file_contents.at(0) == 0xFF && file_contents.at(1) == 0xD8 &&
            file_contents.at(2) == 0xFF &&
            (file_contents.at(3) == 0xDB || file_contents.at(3) == 0xE0))
            return "image/jpeg";

        // "[Iridium]"
        if (file_contents.at(0) == 0x5B && file_contents.at(1) == 0x49 &&
            file_contents.at(2) == 0x52 && file_contents.at(3) == 0x43 &&
            file_contents.at(4) == 0x4E && file_contents.at(5) == 0x46 &&
            file_contents.at(6) == 0x5B)
            // Non-standard MIME, but the engine uses it.
            return "text/iridium-config";

        return "application/octet-stream";
    }
}
