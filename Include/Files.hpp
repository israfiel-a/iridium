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

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

/**
 * @brief The Iridium namespace.
 */
namespace Iridium
{
    /**
     * @brief A file object. This contains methods for retrieving file
     * metadata, getting the contents of a file in different formats, and
     * more.
     */
    class File
    {
        private:
            /**
             * @brief The directory within the application's asset
             * directory that this file is located within.
             */
            std::string directory;

            /**
             * @brief The basename of the file.
             */
            std::string basename;

            /**
             * @brief A vector of raw bytes that represents the
             * contents of the file.
             */
            std::vector<std::uint8_t> contents;

        public:
            /**
             * @brief Load a file from the given path.
             * @param path The path to the file within the game's asset
             * directory.
             */
            File(const std::string &path);

            /**
             * @brief Get the directory within the game's asset directory
             * that this file exists in.
             * @return The directory name.
             */
            const std::string &GetDirectory() const;

            /**
             * @brief Get the name of the file, minus the path and
             * extension.
             * @return The file's basename.
             */
            const std::string &GetBasename() const;

            /**
             * @brief Get the type of the file.
             * @return The type of the file.
             */
            std::string GetType() const;

            /**
             * @brief Get the size of the file in bytes.
             * @return The size of the file.
             */
            std::ifstream::pos_type GetSize() const;

            /**
             * @brief Get the contents of a file as raw bytes.
             * @return A vector of the file's raw bytes.
             */
            const std::vector<std::uint8_t> &GetContents() const;

            /**
             * @brief Get the contents of the file cast to a string. This
             * will attempt to coerce the raw bytes into a string, but note
             * that this can produce strange output depending on
             * formatting.
             * @return The string representation of the file.
             */
            std::string GetContentsAsString() const;

            /**
             * @brief Hexdump the file to a specified output stream.
             * @param output The ouput stream to print the hexdump to.
             * @param column_count The amount of horizontal columns to
             * print.
             * @param metadata A boolean that represents whether or not to
             * print the file metadata.
             * @param linenum A boolean that represents whether or not to
             * print line numbers alongside file contents.
             * @param characters A boolean that represents whether or not
             * to print ASCII transliterations of each byte should there be
             * one.
             */
            void Hexdump(const std::ostream &output = std::cout,
                         std::uint32_t column_count = 32,
                         bool metadata = true, bool linenum = true,
                         bool characters = true) const;
    };

    std::string GetApplicationAssetDirectory();

    std::string GetAssetSubdirectoryFromPath(const std::string &path);

    std::string GetBasenameFromPath(const std::string &path);

    std::string InferFiletype(const std::vector<uint8_t> &file_contents);
}

#endif // IRIDIUM_FILES_HPP
