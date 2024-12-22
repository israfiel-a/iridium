/**
 * @file Files.hpp
 * @author Israfil Argos (israfiel-a)
 * @date 2024-12-16
 * @brief A file providing functionality for loading and understanding
 * files.
 *
 * Copyright (c) 2024 Israfil Argos
 * This file is under the AGPLv3. For more information on what that
 * entails, see the LICENSE file provided with the engine.
 */
#ifndef IRIDIUM_FILES_HPP
#define IRIDIUM_FILES_HPP

// C++ standard I/O operations.
#include <iostream>
// C++ filesystem library.
#include <filesystem>
// C++ standard variable-sized array.
#include <vector>

/**
 * @brief The Iridium namespace.
 */
namespace Iridium
{
    /**
     * @brief The path to a file. This is an STL filesystem::path
     * structure.
     */
    typedef std::filesystem::path FilePath;
    /**
     * @brief A file's contents. This is an STL vector of unsigned bytes.
     */
    typedef std::vector<std::uint8_t> FileContents;

    /**
     * @brief Utilities having to do with MIME type.
     */
    namespace MIME
    {
        /**
         * @brief A file's MIME type. This is an STL string object.
         */
        typedef std::string FileMIME;

        /**
         * @brief The MIME for a JPEG file. The signature for this is `FF
         * D8 FF (D8 | E0)`.
         */
        constexpr const char *jpeg_mime = "image/jpeg";

        /**
         * @brief The MIME for a PNG file. The signature for this is `89 50
         * 4E 47 0D 0A 1A 0A`.
         */
        constexpr const char *png_mime = "image/png";

        /**
         * @brief The MIME for an Iridium config file. Since each one
         * begins with an [Iridium], the signature is just that. This is
         * not standard.
         */
        constexpr const char *config_mime = "text/config";

        /**
         * @brief The MIME for a generic/unknown file.
         */
        constexpr const char *generic_mime = "application/octet-stream";

        FileMIME GetFiletype(const FileContents &file_contents);
    }

    /**
     * @brief A file object. This is not for writing, it's just for
     * reading. For writing, see MutableFile.
     */
    class File
    {
        private:
            /**
             * @brief The path to the file.
             */
            FilePath path;

            /**
             * @brief A vector of raw bytes that represents the
             * contents of the file.
             */
            FileContents contents;

        public:
            /**
             * @brief Load a file from the given path.
             * @param path The path to the file within the game's asset
             * directory.
             */
            File(const FilePath &path);

            /**
             * @brief Get the directory within the game's asset directory
             * that this file exists in.
             * @return The directory name.
             */
            inline const FilePath &GetPath() const noexcept
            {
                return this->path;
            }

            /**
             * @brief Get the basename of the file.
             * @return The file's basename.
             */
            inline FilePath GetBasename() const noexcept
            {
                return this->path.stem();
            }

            /**
             * @brief Get the file's extension. This is NOT the file's MIME
             * type.
             * @return The file's extension.
             */
            inline FilePath GetExtension() const noexcept
            {
                return this->path.extension();
            }

            /**
             * @brief Get the contents of a file as raw bytes.
             * @return A vector of the file's raw bytes.
             */
            inline const FileContents &GetContents() const noexcept
            {
                return this->contents;
            }

            /**
             * @brief Get the size of the file in bytes.
             * @return The size of the file.
             */
            inline std::size_t GetSize() const noexcept
            {
                return this->contents.size();
            }

            /**
             * @brief Get the type of the file.
             * @return The type of the file.
             */
            inline MIME::FileMIME GetType() const noexcept
            {
                return MIME::GetFiletype(this->GetContents());
            }

            /**
             * @brief Stringify the file's metadata.
             * @return The stringified data.
             */
            const std::string &StringifyMetadata() const noexcept;

            /**
             * @brief Get the contents of the file in string format.
             * @return The string of file content.
             */
            inline std::string Stringify() const noexcept
            {
                return std::string(this->contents.begin(),
                                   this->contents.end());
            }

            /**
             * @brief Hexdump the file to a specified output stream. This
             * flushes the output stream.
             * @param output The ouput stream to print the hexdump to.
             * @param column_count The amount of horizontal columns to
             * print.
             * @param characters A boolean that represents whether or not
             * to print ASCII transliterations of each byte should there be
             * one.
             */
            void Hexdump(std::ostream &output = std::cout,
                         std::uint32_t column_count = 32,
                         bool characters = true) const;
    };

    /**
     * @brief Attempt to normalize (remove things like root-traversal,
     * double separators, etc.) a given path. This will merge the file path
     * with the application's asset directory.
     * @param path The path to normalize.
     * @return The normalized path.
     */
    FilePath NormalizePath(const FilePath &path);

    /**
     * @brief Get the asset directory.
     * @return The asset directory's path.
     * @note This function is effectively boilerplate at this time; it
     * exists only for the future, where configuring this value will be
     * possible.
     */
    FilePath GetAssetDirectory();
}

#endif // IRIDIUM_FILES_HPP