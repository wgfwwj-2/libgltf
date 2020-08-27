/*
 * This software is released under the MIT license.
 * 
 * Copyright (c) 2017-2020 Alex Chi, The Code 4 Game Organization
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
 
 #include "runtest.h"

#include <string>
#include <sstream>
#include <fstream>

#if defined(LIBGLTF_PLATFORM_WINDOWS)
#include <tchar.h>
#include <crtdbg.h>
#endif

void SaveAsOBJ(const std::string& _sFilePath
    , const libgltf::TDimensionVector<1, size_t>& _TriangleIndices
    , const libgltf::TDimensionVector<3, float>& _VertexPositions
    , const libgltf::TDimensionVector<2, float>& _VertexTexcoord
    , const libgltf::TDimensionVector<3, float>& _VertexNormal
    , float _Scale = 100.0f)
{
    std::ofstream obj_stream(_sFilePath.c_str());
    if (!obj_stream.good()) return;

    obj_stream << "# generated by libgltf" << std::endl;

    obj_stream << "# vertex" << std::endl;
    for (size_t i = 0; i < _VertexPositions.size(); ++i)
    {
        const libgltf::TDimensionVector<3, float>::TValue& position_item = _VertexPositions[i];
        obj_stream << "v " << position_item[0] * _Scale << " " << position_item[1] * _Scale << " " << position_item[2] * _Scale << std::endl;
    }
    obj_stream << std::endl;

    obj_stream << "# texcoord" << std::endl;
    for (size_t i = 0; i < _VertexTexcoord.size(); ++i)
    {
        const libgltf::TDimensionVector<2, float>::TValue& texcoord_0_item = _VertexTexcoord[i];
        obj_stream << "vt " << texcoord_0_item[0] << " " << texcoord_0_item[1] << std::endl;
    }
    obj_stream << std::endl;

    obj_stream << "# normal" << std::endl;
    for (size_t i = 0; i < _VertexNormal.size(); ++i)
    {
        const libgltf::TDimensionVector<3, float>::TValue& normal_item = _VertexNormal[i];
        obj_stream << "vn " << normal_item[0] << " " << normal_item[1] << " " << normal_item[2] << std::endl;
    }
    obj_stream << std::endl;

    obj_stream << "# face" << std::endl;
    for (size_t i = 0; i < _TriangleIndices.size(); i += 3)
    {
        obj_stream << "f";
        for (size_t j = 0; j < 3; ++j)
        {
            size_t triangle_item = _TriangleIndices[i + j][0];
            if (triangle_item >= _VertexPositions.size())
            {
                break;
            }
            ++triangle_item;
            obj_stream << " " << triangle_item << "/" << triangle_item << "/" << triangle_item;
        }
        obj_stream << std::endl;
    }
    obj_stream.close();
}

#if defined(LIBGLTF_CHARACTOR_ENCODING_IS_UNICODE) && defined(LIBGLTF_PLATFORM_WINDOWS)
int _tmain(int _iArgc, wchar_t* _pcArgv[])
#else
int main(int _iArgc, char* _pcArgv[])
#endif
{
#if defined(LIBGLTF_PLATFORM_WINDOWS) && defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    int error_code = 1;

#if defined(LIBGLTF_CHARACTOR_ENCODING_IS_UNICODE) && defined(LIBGLTF_PLATFORM_WINDOWS)
    std::wstring input_file_path;
#else
    std::string input_file_path;
#endif
    for (int i = 1; i < _iArgc; ++i)
    {
#if defined(LIBGLTF_CHARACTOR_ENCODING_IS_UNICODE) && defined(LIBGLTF_PLATFORM_WINDOWS)
        std::wstringstream argument;
#else
        std::stringstream argument;
#endif
        argument << _pcArgv[i];
#if defined(LIBGLTF_CHARACTOR_ENCODING_IS_UNICODE) && defined(LIBGLTF_PLATFORM_WINDOWS)
        if (argument.str() == L"--coveralls")
#else
        if (argument.str() == "--coveralls")
#endif
        {
            error_code = 0;
        }
        else
        {
            input_file_path = argument.str();
        }
    }

    if (input_file_path.length() == 0)
    {
#if defined(LIBGLTF_PLATFORM_WINDOWS) && defined(LIBGLTF_CHARACTOR_ENCODING_IS_UNICODE)
        wprintf(L"Command line format: runtest [--coveralls] input_file_path\n");
#else
        printf("Command line format: runtest [--coveralls] input_file_path\n");
#endif
        return error_code;
    }

#if defined(LIBGLTF_CHARACTOR_ENCODING_IS_UTF16)
    std::shared_ptr<libgltf::IglTFLoader> gltf_loader = libgltf::IglTFLoader::Create(libgltf::UTF8ToUTF16(input_file_path));
#elif defined(LIBGLTF_CHARACTOR_ENCODING_IS_UTF32)
    std::shared_ptr<libgltf::IglTFLoader> gltf_loader = libgltf::IglTFLoader::Create(libgltf::UTF8ToUTF32(input_file_path));
#elif defined(LIBGLTF_CHARACTOR_ENCODING_IS_UNICODE)
#if defined(LIBGLTF_PLATFORM_WINDOWS)
    std::shared_ptr<libgltf::IglTFLoader> gltf_loader = libgltf::IglTFLoader::Create(input_file_path);
#else
    std::shared_ptr<libgltf::IglTFLoader> gltf_loader = libgltf::IglTFLoader::Create(libgltf::UTF8ToUNICODE(input_file_path));
#endif
#else
    std::shared_ptr<libgltf::IglTFLoader> gltf_loader = libgltf::IglTFLoader::Create(input_file_path);
#endif
    
    std::shared_ptr<libgltf::SGlTF> loaded_gltf = gltf_loader->glTF().lock();
    if (loaded_gltf)
    {
        printf("operator << Success\n");
    }
    else
    {
        printf("operator << Failed\n");
        return error_code;
    }

#if defined(LIBGLTF_CHARACTOR_ENCODING_IS_UNICODE)
    libgltf::TDimensionVector<1, size_t> triangle_data;
    std::shared_ptr<libgltf::TAccessorStream<libgltf::TDimensionVector<1, size_t> > > triangle_stream = std::make_shared<libgltf::TAccessorStream<libgltf::TDimensionVector<1, size_t> > >(triangle_data);
    gltf_loader->GetOrLoadMeshPrimitiveIndicesData(0, 0, triangle_stream);

    libgltf::TDimensionVector<3, float> position_data;
    std::shared_ptr<libgltf::TAccessorStream<libgltf::TDimensionVector<3, float> > > position_stream = std::make_shared<libgltf::TAccessorStream<libgltf::TDimensionVector<3, float> > >(position_data);
    gltf_loader->GetOrLoadMeshPrimitiveAttributeData(0, 0, L"position", position_stream);

    libgltf::TDimensionVector<3, float> normal_data;
    std::shared_ptr<libgltf::TAccessorStream<libgltf::TDimensionVector<3, float> > > normal_stream = std::make_shared<libgltf::TAccessorStream<libgltf::TDimensionVector<3, float> > >(normal_data);
    gltf_loader->GetOrLoadMeshPrimitiveAttributeData(0, 0, L"normal", normal_stream);

    libgltf::TDimensionVector<2, float> texcoord_0_data;
    std::shared_ptr<libgltf::TAccessorStream<libgltf::TDimensionVector<2, float> > > texcoord_0_stream = std::make_shared<libgltf::TAccessorStream<libgltf::TDimensionVector<2, float> > >(texcoord_0_data);
    gltf_loader->GetOrLoadMeshPrimitiveAttributeData(0, 0, L"texcoord_0", texcoord_0_stream);
#else
    libgltf::TDimensionVector<1, size_t> triangle_data;
    std::shared_ptr<libgltf::TAccessorStream<libgltf::TDimensionVector<1, size_t> > > triangle_stream = std::make_shared<libgltf::TAccessorStream<libgltf::TDimensionVector<1, size_t> > >(triangle_data);
    gltf_loader->GetOrLoadMeshPrimitiveIndicesData(0, 0, triangle_stream);

    libgltf::TDimensionVector<3, float> position_data;
    std::shared_ptr<libgltf::TAccessorStream<libgltf::TDimensionVector<3, float> > > position_stream = std::make_shared<libgltf::TAccessorStream<libgltf::TDimensionVector<3, float> > >(position_data);
    gltf_loader->GetOrLoadMeshPrimitiveAttributeData(0, 0, "position", position_stream);

    libgltf::TDimensionVector<3, float> normal_data;
    std::shared_ptr<libgltf::TAccessorStream<libgltf::TDimensionVector<3, float> > > normal_stream = std::make_shared<libgltf::TAccessorStream<libgltf::TDimensionVector<3, float> > >(normal_data);
    gltf_loader->GetOrLoadMeshPrimitiveAttributeData(0, 0, "normal", normal_stream);

    libgltf::TDimensionVector<2, float> texcoord_0_data;
    std::shared_ptr<libgltf::TAccessorStream<libgltf::TDimensionVector<2, float> > > texcoord_0_stream = std::make_shared<libgltf::TAccessorStream<libgltf::TDimensionVector<2, float> > >(texcoord_0_data);
    gltf_loader->GetOrLoadMeshPrimitiveAttributeData(0, 0, "texcoord_0", texcoord_0_stream);

    std::vector<uint8_t> image0_data;
    libgltf::string_t image0_data_type;
    gltf_loader->GetOrLoadImageData(0, image0_data, image0_data_type);
#endif

    //TODO: just convert to json, save the mesh or image data to file in future
    libgltf::string_t output_content;
    if (loaded_gltf >> output_content)
    {
        printf("operator >> Success\n");
    }
    else
    {
        printf("operator >> Failed\n");
        return error_code;
    }

    return 0;
}
