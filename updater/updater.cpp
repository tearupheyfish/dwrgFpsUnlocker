//
// Created by Tofu on 25-6-23.
//

#include <unzip.h>

#include <iostream>
#include <filesystem>
#include <fstream>
#include <array>


#define MAX_FILENAME 260
#define READ_SIZE 8192
bool unzipFile(const std::filesystem::path &zipPath, const std::filesystem::path &outputDir) {

    auto file = unzOpen(zipPath.string().c_str());
    if(!file) {
        std::cerr<<"无法打开压缩包："<<zipPath<<std::endl;
        return false;
    }
    unz_global_info info;
    if(unzGetGlobalInfo(file, &info) != UNZ_OK) {
        std::cerr<<"无法读取压缩包信息"<<std::endl;
        unzClose(file);
        return false;
    }

    uLong i;
    for ( i = 0; i < info.number_entry; ++i )
    {
        // 当前文件信息
        unz_file_info cur_file_info;
        char filename[ MAX_FILENAME ];
        if ( unzGetCurrentFileInfo(
                file,
                &cur_file_info,
                filename,
                MAX_FILENAME,
                NULL, 0, NULL, 0 ) != UNZ_OK )
        {
            std::cout<<"无法获取文件信息："<<filename<<std::endl;
            unzClose( file );
            return false;
        }

        // 确定filename是否为文件夹
        const size_t filename_length = strlen( filename );
        if (filename[filename_length-1] == '\\' || filename[filename_length-1] == '/')
        {
            // Entry is a directory, so create it.
            std::cout<<"目录:"<<filename<<std::endl;
            std::filesystem::create_directory(outputDir/filename);
        }
        else
        {
            std::cout<<"文件:"<<filename<<std::endl;
            if ( unzOpenCurrentFile( file ) != UNZ_OK )
            {
                std::cout<<"\t无法打开文件"<<std::endl;
                unzClose( file );
                return false;
            }

            const auto outputfilepath = outputDir/filename;

            std::ofstream out(outputfilepath, std::ios::binary);
            if (!out.is_open()) {  // 确保文件成功打开
                std::cerr << "\t无法创建文件：" << outputfilepath << std::endl;
                unzCloseCurrentFile(file);
                unzClose(file);
                return false;
            }

            int error = UNZ_OK;
            std::array<char, READ_SIZE> buffer; // 使用std::vector作为缓冲区

            do {
                // 从 ZIP 文件中读取数据到缓冲区
                error = unzReadCurrentFile(file, buffer.data(), static_cast<uint32_t>(buffer.size()));
                if (error < 0) {
                    std::cerr << "读取文件出错: " << error << std::endl;
                    unzCloseCurrentFile(file);
                    unzClose(file);
                    return false;
                }

                // 如果有数据，写入输出文件
                if (error > 0) {
                    out.write(buffer.data(), error);
                    if (!out) {  // 检查写入是否成功
                        std::cerr << "写入文件出错：" << filename << std::endl;
                        unzCloseCurrentFile(file);
                        unzClose(file);
                        return false;
                    }
                }
            } while (error > 0); // 继续读取，直到没有数据

        }

        unzCloseCurrentFile( file );

        // Go the the next entry listed in the zip file.
        if ( ( i+1 ) < info.number_entry )
        {
            if ( unzGoToNextFile( file ) != UNZ_OK )
            {
                std::cout<<"错误：无法读取压缩包中下一个文件"<<std::endl;
                unzClose( file );
                return false;
            }
        }
    }
    return true;
}


int main(int argc, char *argv[])
{
    std::ios::sync_with_stdio(true);

    if (argc != 3) {
        std::cerr << "用法: " << argv[0] << "updater <压缩包路径> <项目目录>" << std::endl;
        return 1;
    }

    std::filesystem::path zipPath = argv[1];
    std::filesystem::path outputDir = argv[2];

    if (!std::filesystem::exists(zipPath)) {
        std::cerr << "Error: Zip file does not exist: " << zipPath << std::endl;
        return 1;
    }

    auto unzipdir = zipPath.parent_path()/zipPath.stem();

    if( !std::filesystem::exists(unzipdir) || std::filesystem::is_directory(unzipdir) ) {
        std::cout<<"创建解压目录："<<unzipdir<<std::endl;
        std::filesystem::create_directory(unzipdir);
    }

    if(!unzipFile(zipPath, unzipdir)) {
        return 1;
    }

    if(!std::filesystem::exists(outputDir)) {
        std::cerr<<"项目目录不存在？！"<<outputDir<<std::endl;
        std::filesystem::remove_all(unzipdir);
        return 2;
    }

// Copy all files from unzipdir to outputDir with overwrite
std::cout<<"拷贝覆盖...\n";
    try {
        for (const auto &entry: std::filesystem::recursive_directory_iterator(unzipdir)) {
            auto relativePath = std::filesystem::relative(entry.path(), unzipdir);
            auto targetPath = outputDir / relativePath;

            if (entry.is_directory()) {
                std::filesystem::create_directories(targetPath);
                std::cout<<"拷贝文件："<<relativePath<<'\n';
            } else {
                if (std::filesystem::exists(targetPath)) {
                    std::filesystem::remove(targetPath);
                }
                std::filesystem::copy_file(entry.path(), targetPath);
                std::cout<<"拷贝目录："<<relativePath<<'\n';
            }
        }

        // Clean up temporary directory
        std::cout<<"清理中...\n";
        std::filesystem::remove_all(unzipdir);
    } catch (const std::filesystem::filesystem_error &e) {
        std::cerr << "Error moving files: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}