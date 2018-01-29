#include "libgltfpch.h"
#include "libgltfparser.h"
#include "libgltf.h"

namespace libgltf
{
    class GLTFCharDocumentPtrWrapper
    {
    public:
        explicit GLTFCharDocumentPtrWrapper(GLTFCharDocument*& _pDocument) : m_pDocument(_pDocument) { }
        ~GLTFCharDocumentPtrWrapper() { m_pDocument = nullptr; }
    private:
        GLTFCharDocument*& m_pDocument;
    };
    GLTFCharDocument* g_json_doc_ptr = nullptr;

    bool operator<<(std::shared_ptr<SGlTF>& _pGlTF, const GLTFString& _sContent)
    {
        GLTFCharDocument json_doc;
        json_doc.Parse(_sContent.c_str());
        if (!json_doc.IsObject()) return false;
        return (_pGlTF << json_doc.GetObject());
    }

    bool operator>>(const std::shared_ptr<SGlTF>& _pGlTF, GLTFString& _sContent)
    {
        if (!_pGlTF) return false;
        GLTFCharDocument json_doc;
        g_json_doc_ptr = &json_doc;
        GLTFCharDocumentPtrWrapper wrapper(g_json_doc_ptr);
        if (!(_pGlTF >> json_doc)) return false;
        GLTFStringBuffer json_string_buffer;
        GLTFWriter json_writer(json_string_buffer);
        if (!json_doc.Accept(json_writer)) return false;
        _sContent = json_string_buffer.GetString();
        return true;
    }

    bool operator<<(bool& _rData, const GLTFCharValue& _JsonValue)
    {
        if (!_JsonValue.IsBool()) return false;
        _rData = _JsonValue.GetBool();
        return true;
    }

    bool operator>>(bool _bData, GLTFCharValue& _JsonValue)
    {
        _JsonValue.SetBool(_bData);
        return true;
    }

    bool operator<<(int32_t& _rData, const GLTFCharValue& _JsonValue)
    {
        if (!_JsonValue.IsInt()) return false;
        _rData = _JsonValue.GetInt();
        return true;
    }

    bool operator>>(int32_t _iData, GLTFCharValue& _JsonValue)
    {
        _JsonValue.SetInt(_iData);
        return true;
    }

    bool operator<<(float& _rData, const GLTFCharValue& _JsonValue)
    {
        if (_JsonValue.IsFloat())
        {
            _rData = _JsonValue.GetFloat();
            return true;
        }
        if (_JsonValue.IsInt())
        {
            _rData = static_cast<float>(_JsonValue.GetInt());
            return true;
        }
        return false;
    }

    bool operator>>(float _fData, GLTFCharValue& _JsonValue)
    {
        _JsonValue.SetFloat(_fData);
        return true;
    }

    bool operator<<(GLTFString& _rData, const GLTFCharValue& _JsonValue)
    {
        if (!_JsonValue.IsString()) return false;
        _rData = _JsonValue.GetString();
        return true;
    }

    bool operator>>(const GLTFString& _rsData, GLTFCharValue& _JsonValue)
    {
        if (!g_json_doc_ptr) return false;
        _JsonValue.SetString(_rsData.c_str(), g_json_doc_ptr->GetAllocator());
        return true;
    }

    template<typename TData>
    bool operator<<(std::vector<TData>& _vDatas, const GLTFCharValue& _JsonValue)
    {
        if (!_JsonValue.IsArray()) return false;
        std::vector<TData> datas;
        const GLTFCharConstArray& json_array = _JsonValue.GetArray();
        size_t len = json_array.Size();
        if (len == 0) return true;
        datas.resize(len);
        for (size_t i = 0; i < len; ++i) if (!(datas[i] << json_array[static_cast<rapidjson::SizeType>(i)])) return false;
        _vDatas = datas;
        return true;
    }

    template<typename TData>
    bool operator>>(const std::vector<TData>& _vDatas, GLTFCharValue& _JsonValue)
    {
        if (_vDatas.empty() || !g_json_doc_ptr) return false;
        GLTFCharValue& json_array = _JsonValue.SetArray();
        for (const TData& data : _vDatas)
        {
            GLTFCharValue json_value;
            if (!(data >> json_value)) return false;
            json_array.PushBack(json_value, g_json_doc_ptr->GetAllocator());
        }
        return true;
    }

    template<typename TData>
    bool operator<<(std::map<GLTFString, TData>& _mDatas, const GLTFCharValue& _JsonValue)
    {
        if (!_JsonValue.IsObject()) return false;
        std::map<GLTFString, TData> datas;
        const GLTFCharConstObject& json_object = _JsonValue.GetObject();
        for (GLTFCharConstObject::ConstMemberIterator cit = json_object.MemberBegin(); cit != json_object.MemberEnd(); ++cit)
        {
            TData data;
            if (!(data << cit->value)) return false;
            datas.insert(std::make_pair(cit->name.GetString(), data));
        }
        _mDatas = datas;
        return true;
    }

    template<typename TData>
    bool operator>>(const std::map<GLTFString, TData>& _mDatas, GLTFCharValue& _JsonValue)
    {
        if (_mDatas.empty() || !g_json_doc_ptr) return false;
        _JsonValue.SetObject();
        for (const std::pair<GLTFString, TData>& data : _mDatas)
        {
            GLTFCharValue json_value;
            if (!(data.second >> json_value)) return false;
            GLTFCharValue json_key(data.first.c_str(), g_json_doc_ptr->GetAllocator());
            _JsonValue.AddMember(json_key, json_value, g_json_doc_ptr->GetAllocator());
        }
        return true;
    }

    bool operator<<(std::shared_ptr<SGlTFProperty>& _pData, const GLTFCharValue& _JsonValue)
    {
        std::shared_ptr<SGlTFProperty> data_ptr = !!_pData ? _pData : std::make_shared<SGlTFProperty>();
        if (_JsonValue.HasMember(GLTFTEXT("extras")) && _JsonValue[GLTFTEXT("extras")].IsObject())
        {
            if (!(data_ptr->extras << _JsonValue[GLTFTEXT("extras")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("extensions")) && _JsonValue[GLTFTEXT("extensions")].IsObject())
        {
            if (!(data_ptr->extensions << _JsonValue[GLTFTEXT("extensions")])) return false;
        }
        _pData = data_ptr;
        return true;
    }

    bool operator>>(const std::shared_ptr<SGlTFProperty>& _pData, GLTFCharValue& _JsonValue)
    {
        if (!_pData || !g_json_doc_ptr) return false;
        _JsonValue.SetObject();
        if (!!_pData->extras)
        {
            GLTFCharValue json_value;
            if (!(_pData->extras >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("extras"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!!_pData->extensions)
        {
            GLTFCharValue json_value;
            if (!(_pData->extensions >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("extensions"), json_value, g_json_doc_ptr->GetAllocator());
        }
        return true;
    }

    bool operator<<(std::vector<std::shared_ptr<SGlTFProperty>>& _vDatas, const GLTFCharValue& _JsonValue)
    {
        return operator<< <std::shared_ptr<SGlTFProperty>>(_vDatas, _JsonValue);
    }

    bool operator>>(const std::vector<std::shared_ptr<SGlTFProperty>>& _vDatas, GLTFCharValue& _JsonValue)
    {
        return operator>> <std::shared_ptr<SGlTFProperty>>(_vDatas, _JsonValue);
    }

    bool operator<<(std::shared_ptr<SMaterial>& _pData, const GLTFCharValue& _JsonValue)
    {
        std::shared_ptr<SMaterial> data_ptr = !!_pData ? _pData : std::make_shared<SMaterial>();
        {
            std::shared_ptr<SGlTFChildofRootProperty> super_ptr = data_ptr;
            if (!(super_ptr << _JsonValue)) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("alphaCutoff")) && _JsonValue[GLTFTEXT("alphaCutoff")].IsFloat())
        {
            data_ptr->alphaCutoff = _JsonValue[GLTFTEXT("alphaCutoff")].GetFloat();
        }
        if (_JsonValue.HasMember(GLTFTEXT("emissiveTexture")) && _JsonValue[GLTFTEXT("emissiveTexture")].IsObject())
        {
            if (!(data_ptr->emissiveTexture << _JsonValue[GLTFTEXT("emissiveTexture")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("pbrMetallicRoughness")) && _JsonValue[GLTFTEXT("pbrMetallicRoughness")].IsObject())
        {
            if (!(data_ptr->pbrMetallicRoughness << _JsonValue[GLTFTEXT("pbrMetallicRoughness")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("occlusionTexture")) && _JsonValue[GLTFTEXT("occlusionTexture")].IsObject())
        {
            if (!(data_ptr->occlusionTexture << _JsonValue[GLTFTEXT("occlusionTexture")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("alphaMode")) && _JsonValue[GLTFTEXT("alphaMode")].IsString())
        {
            data_ptr->alphaMode = _JsonValue[GLTFTEXT("alphaMode")].GetString();
        }
        if (_JsonValue.HasMember(GLTFTEXT("doubleSided")) && _JsonValue[GLTFTEXT("doubleSided")].IsBool())
        {
            data_ptr->doubleSided = _JsonValue[GLTFTEXT("doubleSided")].GetBool();
        }
        if (_JsonValue.HasMember(GLTFTEXT("normalTexture")) && _JsonValue[GLTFTEXT("normalTexture")].IsObject())
        {
            if (!(data_ptr->normalTexture << _JsonValue[GLTFTEXT("normalTexture")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("emissiveFactor")) && _JsonValue[GLTFTEXT("emissiveFactor")].IsArray())
        {
            if (!(data_ptr->emissiveFactor << _JsonValue[GLTFTEXT("emissiveFactor")])) return false;
        }
        _pData = data_ptr;
        return true;
    }

    bool operator>>(const std::shared_ptr<SMaterial>& _pData, GLTFCharValue& _JsonValue)
    {
        if (!_pData || !g_json_doc_ptr) return false;
        _JsonValue.SetObject();
        {
            const std::shared_ptr<SGlTFChildofRootProperty> super_ptr = _pData;
            if (!(super_ptr >> _JsonValue)) return false;
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->alphaCutoff >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("alphaCutoff"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!!_pData->emissiveTexture)
        {
            GLTFCharValue json_value;
            if (!(_pData->emissiveTexture >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("emissiveTexture"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!!_pData->pbrMetallicRoughness)
        {
            GLTFCharValue json_value;
            if (!(_pData->pbrMetallicRoughness >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("pbrMetallicRoughness"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!!_pData->occlusionTexture)
        {
            GLTFCharValue json_value;
            if (!(_pData->occlusionTexture >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("occlusionTexture"), json_value, g_json_doc_ptr->GetAllocator());
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->alphaMode >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("alphaMode"), json_value, g_json_doc_ptr->GetAllocator());
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->doubleSided >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("doubleSided"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!!_pData->normalTexture)
        {
            GLTFCharValue json_value;
            if (!(_pData->normalTexture >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("normalTexture"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!_pData->emissiveFactor.empty())
        {
            GLTFCharValue json_value;
            if (!(_pData->emissiveFactor >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("emissiveFactor"), json_value, g_json_doc_ptr->GetAllocator());
        }
        return true;
    }

    bool operator<<(std::vector<std::shared_ptr<SMaterial>>& _vDatas, const GLTFCharValue& _JsonValue)
    {
        return operator<< <std::shared_ptr<SMaterial>>(_vDatas, _JsonValue);
    }

    bool operator>>(const std::vector<std::shared_ptr<SMaterial>>& _vDatas, GLTFCharValue& _JsonValue)
    {
        return operator>> <std::shared_ptr<SMaterial>>(_vDatas, _JsonValue);
    }

    bool operator<<(std::shared_ptr<SAsset>& _pData, const GLTFCharValue& _JsonValue)
    {
        std::shared_ptr<SAsset> data_ptr = !!_pData ? _pData : std::make_shared<SAsset>();
        {
            std::shared_ptr<SGlTFProperty> super_ptr = data_ptr;
            if (!(super_ptr << _JsonValue)) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("minVersion")) && _JsonValue[GLTFTEXT("minVersion")].IsString())
        {
            data_ptr->minVersion = _JsonValue[GLTFTEXT("minVersion")].GetString();
        }
        if (_JsonValue.HasMember(GLTFTEXT("version")) && _JsonValue[GLTFTEXT("version")].IsString())
        {
            data_ptr->version = _JsonValue[GLTFTEXT("version")].GetString();
        }
        if (_JsonValue.HasMember(GLTFTEXT("generator")) && _JsonValue[GLTFTEXT("generator")].IsString())
        {
            data_ptr->generator = _JsonValue[GLTFTEXT("generator")].GetString();
        }
        if (_JsonValue.HasMember(GLTFTEXT("copyright")) && _JsonValue[GLTFTEXT("copyright")].IsString())
        {
            data_ptr->copyright = _JsonValue[GLTFTEXT("copyright")].GetString();
        }
        _pData = data_ptr;
        return true;
    }

    bool operator>>(const std::shared_ptr<SAsset>& _pData, GLTFCharValue& _JsonValue)
    {
        if (!_pData || !g_json_doc_ptr) return false;
        _JsonValue.SetObject();
        {
            const std::shared_ptr<SGlTFProperty> super_ptr = _pData;
            if (!(super_ptr >> _JsonValue)) return false;
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->minVersion >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("minVersion"), json_value, g_json_doc_ptr->GetAllocator());
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->version >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("version"), json_value, g_json_doc_ptr->GetAllocator());
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->generator >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("generator"), json_value, g_json_doc_ptr->GetAllocator());
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->copyright >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("copyright"), json_value, g_json_doc_ptr->GetAllocator());
        }
        return true;
    }

    bool operator<<(std::vector<std::shared_ptr<SAsset>>& _vDatas, const GLTFCharValue& _JsonValue)
    {
        return operator<< <std::shared_ptr<SAsset>>(_vDatas, _JsonValue);
    }

    bool operator>>(const std::vector<std::shared_ptr<SAsset>>& _vDatas, GLTFCharValue& _JsonValue)
    {
        return operator>> <std::shared_ptr<SAsset>>(_vDatas, _JsonValue);
    }

    bool operator<<(std::shared_ptr<SSampler>& _pData, const GLTFCharValue& _JsonValue)
    {
        std::shared_ptr<SSampler> data_ptr = !!_pData ? _pData : std::make_shared<SSampler>();
        {
            std::shared_ptr<SGlTFChildofRootProperty> super_ptr = data_ptr;
            if (!(super_ptr << _JsonValue)) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("wrapS")) && _JsonValue[GLTFTEXT("wrapS")].IsInt())
        {
            data_ptr->wrapS = _JsonValue[GLTFTEXT("wrapS")].GetInt();
        }
        if (_JsonValue.HasMember(GLTFTEXT("minFilter")) && _JsonValue[GLTFTEXT("minFilter")].IsInt())
        {
            data_ptr->minFilter = _JsonValue[GLTFTEXT("minFilter")].GetInt();
        }
        if (_JsonValue.HasMember(GLTFTEXT("magFilter")) && _JsonValue[GLTFTEXT("magFilter")].IsInt())
        {
            data_ptr->magFilter = _JsonValue[GLTFTEXT("magFilter")].GetInt();
        }
        if (_JsonValue.HasMember(GLTFTEXT("wrapT")) && _JsonValue[GLTFTEXT("wrapT")].IsInt())
        {
            data_ptr->wrapT = _JsonValue[GLTFTEXT("wrapT")].GetInt();
        }
        _pData = data_ptr;
        return true;
    }

    bool operator>>(const std::shared_ptr<SSampler>& _pData, GLTFCharValue& _JsonValue)
    {
        if (!_pData || !g_json_doc_ptr) return false;
        _JsonValue.SetObject();
        {
            const std::shared_ptr<SGlTFChildofRootProperty> super_ptr = _pData;
            if (!(super_ptr >> _JsonValue)) return false;
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->wrapS >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("wrapS"), json_value, g_json_doc_ptr->GetAllocator());
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->minFilter >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("minFilter"), json_value, g_json_doc_ptr->GetAllocator());
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->magFilter >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("magFilter"), json_value, g_json_doc_ptr->GetAllocator());
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->wrapT >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("wrapT"), json_value, g_json_doc_ptr->GetAllocator());
        }
        return true;
    }

    bool operator<<(std::vector<std::shared_ptr<SSampler>>& _vDatas, const GLTFCharValue& _JsonValue)
    {
        return operator<< <std::shared_ptr<SSampler>>(_vDatas, _JsonValue);
    }

    bool operator>>(const std::vector<std::shared_ptr<SSampler>>& _vDatas, GLTFCharValue& _JsonValue)
    {
        return operator>> <std::shared_ptr<SSampler>>(_vDatas, _JsonValue);
    }

    bool operator<<(std::shared_ptr<SGlTFChildofRootProperty>& _pData, const GLTFCharValue& _JsonValue)
    {
        std::shared_ptr<SGlTFChildofRootProperty> data_ptr = !!_pData ? _pData : std::make_shared<SGlTFChildofRootProperty>();
        {
            std::shared_ptr<SGlTFProperty> super_ptr = data_ptr;
            if (!(super_ptr << _JsonValue)) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("name")) && _JsonValue[GLTFTEXT("name")].IsString())
        {
            data_ptr->name = _JsonValue[GLTFTEXT("name")].GetString();
        }
        _pData = data_ptr;
        return true;
    }

    bool operator>>(const std::shared_ptr<SGlTFChildofRootProperty>& _pData, GLTFCharValue& _JsonValue)
    {
        if (!_pData || !g_json_doc_ptr) return false;
        _JsonValue.SetObject();
        {
            const std::shared_ptr<SGlTFProperty> super_ptr = _pData;
            if (!(super_ptr >> _JsonValue)) return false;
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->name >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("name"), json_value, g_json_doc_ptr->GetAllocator());
        }
        return true;
    }

    bool operator<<(std::vector<std::shared_ptr<SGlTFChildofRootProperty>>& _vDatas, const GLTFCharValue& _JsonValue)
    {
        return operator<< <std::shared_ptr<SGlTFChildofRootProperty>>(_vDatas, _JsonValue);
    }

    bool operator>>(const std::vector<std::shared_ptr<SGlTFChildofRootProperty>>& _vDatas, GLTFCharValue& _JsonValue)
    {
        return operator>> <std::shared_ptr<SGlTFChildofRootProperty>>(_vDatas, _JsonValue);
    }

    bool operator<<(std::shared_ptr<SAnimationSampler>& _pData, const GLTFCharValue& _JsonValue)
    {
        std::shared_ptr<SAnimationSampler> data_ptr = !!_pData ? _pData : std::make_shared<SAnimationSampler>();
        {
            std::shared_ptr<SGlTFProperty> super_ptr = data_ptr;
            if (!(super_ptr << _JsonValue)) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("input")) && _JsonValue[GLTFTEXT("input")].IsInt())
        {
            if (!(data_ptr->input << _JsonValue[GLTFTEXT("input")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("output")) && _JsonValue[GLTFTEXT("output")].IsInt())
        {
            if (!(data_ptr->output << _JsonValue[GLTFTEXT("output")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("interpolation")) && _JsonValue[GLTFTEXT("interpolation")].IsString())
        {
            data_ptr->interpolation = _JsonValue[GLTFTEXT("interpolation")].GetString();
        }
        _pData = data_ptr;
        return true;
    }

    bool operator>>(const std::shared_ptr<SAnimationSampler>& _pData, GLTFCharValue& _JsonValue)
    {
        if (!_pData || !g_json_doc_ptr) return false;
        _JsonValue.SetObject();
        {
            const std::shared_ptr<SGlTFProperty> super_ptr = _pData;
            if (!(super_ptr >> _JsonValue)) return false;
        }
        if (!!_pData->input)
        {
            GLTFCharValue json_value;
            if (!(_pData->input >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("input"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!!_pData->output)
        {
            GLTFCharValue json_value;
            if (!(_pData->output >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("output"), json_value, g_json_doc_ptr->GetAllocator());
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->interpolation >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("interpolation"), json_value, g_json_doc_ptr->GetAllocator());
        }
        return true;
    }

    bool operator<<(std::vector<std::shared_ptr<SAnimationSampler>>& _vDatas, const GLTFCharValue& _JsonValue)
    {
        return operator<< <std::shared_ptr<SAnimationSampler>>(_vDatas, _JsonValue);
    }

    bool operator>>(const std::vector<std::shared_ptr<SAnimationSampler>>& _vDatas, GLTFCharValue& _JsonValue)
    {
        return operator>> <std::shared_ptr<SAnimationSampler>>(_vDatas, _JsonValue);
    }

    bool operator<<(std::shared_ptr<SExtras>& _pData, const GLTFCharValue& _JsonValue)
    {
        std::shared_ptr<SExtras> data_ptr = !!_pData ? _pData : std::make_shared<SExtras>();
        _pData = data_ptr;
        return true;
    }

    bool operator>>(const std::shared_ptr<SExtras>& _pData, GLTFCharValue& _JsonValue)
    {
        if (!_pData || !g_json_doc_ptr) return false;
        _JsonValue.SetObject();
        return true;
    }

    bool operator<<(std::vector<std::shared_ptr<SExtras>>& _vDatas, const GLTFCharValue& _JsonValue)
    {
        return operator<< <std::shared_ptr<SExtras>>(_vDatas, _JsonValue);
    }

    bool operator>>(const std::vector<std::shared_ptr<SExtras>>& _vDatas, GLTFCharValue& _JsonValue)
    {
        return operator>> <std::shared_ptr<SExtras>>(_vDatas, _JsonValue);
    }

    bool operator<<(std::shared_ptr<SScene>& _pData, const GLTFCharValue& _JsonValue)
    {
        std::shared_ptr<SScene> data_ptr = !!_pData ? _pData : std::make_shared<SScene>();
        {
            std::shared_ptr<SGlTFChildofRootProperty> super_ptr = data_ptr;
            if (!(super_ptr << _JsonValue)) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("nodes")) && _JsonValue[GLTFTEXT("nodes")].IsArray())
        {
            if (!(data_ptr->nodes << _JsonValue[GLTFTEXT("nodes")])) return false;
        }
        _pData = data_ptr;
        return true;
    }

    bool operator>>(const std::shared_ptr<SScene>& _pData, GLTFCharValue& _JsonValue)
    {
        if (!_pData || !g_json_doc_ptr) return false;
        _JsonValue.SetObject();
        {
            const std::shared_ptr<SGlTFChildofRootProperty> super_ptr = _pData;
            if (!(super_ptr >> _JsonValue)) return false;
        }
        if (!_pData->nodes.empty())
        {
            GLTFCharValue json_value;
            if (!(_pData->nodes >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("nodes"), json_value, g_json_doc_ptr->GetAllocator());
        }
        return true;
    }

    bool operator<<(std::vector<std::shared_ptr<SScene>>& _vDatas, const GLTFCharValue& _JsonValue)
    {
        return operator<< <std::shared_ptr<SScene>>(_vDatas, _JsonValue);
    }

    bool operator>>(const std::vector<std::shared_ptr<SScene>>& _vDatas, GLTFCharValue& _JsonValue)
    {
        return operator>> <std::shared_ptr<SScene>>(_vDatas, _JsonValue);
    }

    bool operator<<(std::shared_ptr<SCameraPerspective>& _pData, const GLTFCharValue& _JsonValue)
    {
        std::shared_ptr<SCameraPerspective> data_ptr = !!_pData ? _pData : std::make_shared<SCameraPerspective>();
        {
            std::shared_ptr<SGlTFProperty> super_ptr = data_ptr;
            if (!(super_ptr << _JsonValue)) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("aspectRatio")) && _JsonValue[GLTFTEXT("aspectRatio")].IsFloat())
        {
            data_ptr->aspectRatio = _JsonValue[GLTFTEXT("aspectRatio")].GetFloat();
        }
        if (_JsonValue.HasMember(GLTFTEXT("yfov")) && _JsonValue[GLTFTEXT("yfov")].IsFloat())
        {
            data_ptr->yfov = _JsonValue[GLTFTEXT("yfov")].GetFloat();
        }
        if (_JsonValue.HasMember(GLTFTEXT("znear")) && _JsonValue[GLTFTEXT("znear")].IsFloat())
        {
            data_ptr->znear = _JsonValue[GLTFTEXT("znear")].GetFloat();
        }
        if (_JsonValue.HasMember(GLTFTEXT("zfar")) && _JsonValue[GLTFTEXT("zfar")].IsFloat())
        {
            data_ptr->zfar = _JsonValue[GLTFTEXT("zfar")].GetFloat();
        }
        _pData = data_ptr;
        return true;
    }

    bool operator>>(const std::shared_ptr<SCameraPerspective>& _pData, GLTFCharValue& _JsonValue)
    {
        if (!_pData || !g_json_doc_ptr) return false;
        _JsonValue.SetObject();
        {
            const std::shared_ptr<SGlTFProperty> super_ptr = _pData;
            if (!(super_ptr >> _JsonValue)) return false;
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->aspectRatio >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("aspectRatio"), json_value, g_json_doc_ptr->GetAllocator());
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->yfov >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("yfov"), json_value, g_json_doc_ptr->GetAllocator());
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->znear >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("znear"), json_value, g_json_doc_ptr->GetAllocator());
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->zfar >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("zfar"), json_value, g_json_doc_ptr->GetAllocator());
        }
        return true;
    }

    bool operator<<(std::vector<std::shared_ptr<SCameraPerspective>>& _vDatas, const GLTFCharValue& _JsonValue)
    {
        return operator<< <std::shared_ptr<SCameraPerspective>>(_vDatas, _JsonValue);
    }

    bool operator>>(const std::vector<std::shared_ptr<SCameraPerspective>>& _vDatas, GLTFCharValue& _JsonValue)
    {
        return operator>> <std::shared_ptr<SCameraPerspective>>(_vDatas, _JsonValue);
    }

    bool operator<<(std::shared_ptr<SBufferView>& _pData, const GLTFCharValue& _JsonValue)
    {
        std::shared_ptr<SBufferView> data_ptr = !!_pData ? _pData : std::make_shared<SBufferView>();
        {
            std::shared_ptr<SGlTFChildofRootProperty> super_ptr = data_ptr;
            if (!(super_ptr << _JsonValue)) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("byteLength")) && _JsonValue[GLTFTEXT("byteLength")].IsInt())
        {
            data_ptr->byteLength = _JsonValue[GLTFTEXT("byteLength")].GetInt();
        }
        if (_JsonValue.HasMember(GLTFTEXT("buffer")) && _JsonValue[GLTFTEXT("buffer")].IsInt())
        {
            if (!(data_ptr->buffer << _JsonValue[GLTFTEXT("buffer")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("byteOffset")) && _JsonValue[GLTFTEXT("byteOffset")].IsInt())
        {
            data_ptr->byteOffset = _JsonValue[GLTFTEXT("byteOffset")].GetInt();
        }
        if (_JsonValue.HasMember(GLTFTEXT("target")) && _JsonValue[GLTFTEXT("target")].IsInt())
        {
            data_ptr->target = _JsonValue[GLTFTEXT("target")].GetInt();
        }
        if (_JsonValue.HasMember(GLTFTEXT("byteStride")) && _JsonValue[GLTFTEXT("byteStride")].IsInt())
        {
            data_ptr->byteStride = _JsonValue[GLTFTEXT("byteStride")].GetInt();
        }
        _pData = data_ptr;
        return true;
    }

    bool operator>>(const std::shared_ptr<SBufferView>& _pData, GLTFCharValue& _JsonValue)
    {
        if (!_pData || !g_json_doc_ptr) return false;
        _JsonValue.SetObject();
        {
            const std::shared_ptr<SGlTFChildofRootProperty> super_ptr = _pData;
            if (!(super_ptr >> _JsonValue)) return false;
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->byteLength >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("byteLength"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!!_pData->buffer)
        {
            GLTFCharValue json_value;
            if (!(_pData->buffer >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("buffer"), json_value, g_json_doc_ptr->GetAllocator());
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->byteOffset >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("byteOffset"), json_value, g_json_doc_ptr->GetAllocator());
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->target >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("target"), json_value, g_json_doc_ptr->GetAllocator());
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->byteStride >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("byteStride"), json_value, g_json_doc_ptr->GetAllocator());
        }
        return true;
    }

    bool operator<<(std::vector<std::shared_ptr<SBufferView>>& _vDatas, const GLTFCharValue& _JsonValue)
    {
        return operator<< <std::shared_ptr<SBufferView>>(_vDatas, _JsonValue);
    }

    bool operator>>(const std::vector<std::shared_ptr<SBufferView>>& _vDatas, GLTFCharValue& _JsonValue)
    {
        return operator>> <std::shared_ptr<SBufferView>>(_vDatas, _JsonValue);
    }

    bool operator<<(std::shared_ptr<SMaterialNormalTextureInfo>& _pData, const GLTFCharValue& _JsonValue)
    {
        std::shared_ptr<SMaterialNormalTextureInfo> data_ptr = !!_pData ? _pData : std::make_shared<SMaterialNormalTextureInfo>();
        {
            std::shared_ptr<STextureInfo> super_ptr = data_ptr;
            if (!(super_ptr << _JsonValue)) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("scale")) && _JsonValue[GLTFTEXT("scale")].IsFloat())
        {
            data_ptr->scale = _JsonValue[GLTFTEXT("scale")].GetFloat();
        }
        _pData = data_ptr;
        return true;
    }

    bool operator>>(const std::shared_ptr<SMaterialNormalTextureInfo>& _pData, GLTFCharValue& _JsonValue)
    {
        if (!_pData || !g_json_doc_ptr) return false;
        _JsonValue.SetObject();
        {
            const std::shared_ptr<STextureInfo> super_ptr = _pData;
            if (!(super_ptr >> _JsonValue)) return false;
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->scale >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("scale"), json_value, g_json_doc_ptr->GetAllocator());
        }
        return true;
    }

    bool operator<<(std::vector<std::shared_ptr<SMaterialNormalTextureInfo>>& _vDatas, const GLTFCharValue& _JsonValue)
    {
        return operator<< <std::shared_ptr<SMaterialNormalTextureInfo>>(_vDatas, _JsonValue);
    }

    bool operator>>(const std::vector<std::shared_ptr<SMaterialNormalTextureInfo>>& _vDatas, GLTFCharValue& _JsonValue)
    {
        return operator>> <std::shared_ptr<SMaterialNormalTextureInfo>>(_vDatas, _JsonValue);
    }

    bool operator<<(std::shared_ptr<SMaterialOcclusionTextureInfo>& _pData, const GLTFCharValue& _JsonValue)
    {
        std::shared_ptr<SMaterialOcclusionTextureInfo> data_ptr = !!_pData ? _pData : std::make_shared<SMaterialOcclusionTextureInfo>();
        {
            std::shared_ptr<STextureInfo> super_ptr = data_ptr;
            if (!(super_ptr << _JsonValue)) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("strength")) && _JsonValue[GLTFTEXT("strength")].IsFloat())
        {
            data_ptr->strength = _JsonValue[GLTFTEXT("strength")].GetFloat();
        }
        _pData = data_ptr;
        return true;
    }

    bool operator>>(const std::shared_ptr<SMaterialOcclusionTextureInfo>& _pData, GLTFCharValue& _JsonValue)
    {
        if (!_pData || !g_json_doc_ptr) return false;
        _JsonValue.SetObject();
        {
            const std::shared_ptr<STextureInfo> super_ptr = _pData;
            if (!(super_ptr >> _JsonValue)) return false;
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->strength >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("strength"), json_value, g_json_doc_ptr->GetAllocator());
        }
        return true;
    }

    bool operator<<(std::vector<std::shared_ptr<SMaterialOcclusionTextureInfo>>& _vDatas, const GLTFCharValue& _JsonValue)
    {
        return operator<< <std::shared_ptr<SMaterialOcclusionTextureInfo>>(_vDatas, _JsonValue);
    }

    bool operator>>(const std::vector<std::shared_ptr<SMaterialOcclusionTextureInfo>>& _vDatas, GLTFCharValue& _JsonValue)
    {
        return operator>> <std::shared_ptr<SMaterialOcclusionTextureInfo>>(_vDatas, _JsonValue);
    }

    bool operator<<(std::shared_ptr<SAccessorSparseValues>& _pData, const GLTFCharValue& _JsonValue)
    {
        std::shared_ptr<SAccessorSparseValues> data_ptr = !!_pData ? _pData : std::make_shared<SAccessorSparseValues>();
        {
            std::shared_ptr<SGlTFProperty> super_ptr = data_ptr;
            if (!(super_ptr << _JsonValue)) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("bufferView")) && _JsonValue[GLTFTEXT("bufferView")].IsInt())
        {
            if (!(data_ptr->bufferView << _JsonValue[GLTFTEXT("bufferView")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("byteOffset")) && _JsonValue[GLTFTEXT("byteOffset")].IsInt())
        {
            data_ptr->byteOffset = _JsonValue[GLTFTEXT("byteOffset")].GetInt();
        }
        _pData = data_ptr;
        return true;
    }

    bool operator>>(const std::shared_ptr<SAccessorSparseValues>& _pData, GLTFCharValue& _JsonValue)
    {
        if (!_pData || !g_json_doc_ptr) return false;
        _JsonValue.SetObject();
        {
            const std::shared_ptr<SGlTFProperty> super_ptr = _pData;
            if (!(super_ptr >> _JsonValue)) return false;
        }
        if (!!_pData->bufferView)
        {
            GLTFCharValue json_value;
            if (!(_pData->bufferView >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("bufferView"), json_value, g_json_doc_ptr->GetAllocator());
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->byteOffset >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("byteOffset"), json_value, g_json_doc_ptr->GetAllocator());
        }
        return true;
    }

    bool operator<<(std::vector<std::shared_ptr<SAccessorSparseValues>>& _vDatas, const GLTFCharValue& _JsonValue)
    {
        return operator<< <std::shared_ptr<SAccessorSparseValues>>(_vDatas, _JsonValue);
    }

    bool operator>>(const std::vector<std::shared_ptr<SAccessorSparseValues>>& _vDatas, GLTFCharValue& _JsonValue)
    {
        return operator>> <std::shared_ptr<SAccessorSparseValues>>(_vDatas, _JsonValue);
    }

    bool operator<<(std::shared_ptr<SAnimationChannelTarget>& _pData, const GLTFCharValue& _JsonValue)
    {
        std::shared_ptr<SAnimationChannelTarget> data_ptr = !!_pData ? _pData : std::make_shared<SAnimationChannelTarget>();
        {
            std::shared_ptr<SGlTFProperty> super_ptr = data_ptr;
            if (!(super_ptr << _JsonValue)) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("node")) && _JsonValue[GLTFTEXT("node")].IsInt())
        {
            if (!(data_ptr->node << _JsonValue[GLTFTEXT("node")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("path")) && _JsonValue[GLTFTEXT("path")].IsString())
        {
            data_ptr->path = _JsonValue[GLTFTEXT("path")].GetString();
        }
        _pData = data_ptr;
        return true;
    }

    bool operator>>(const std::shared_ptr<SAnimationChannelTarget>& _pData, GLTFCharValue& _JsonValue)
    {
        if (!_pData || !g_json_doc_ptr) return false;
        _JsonValue.SetObject();
        {
            const std::shared_ptr<SGlTFProperty> super_ptr = _pData;
            if (!(super_ptr >> _JsonValue)) return false;
        }
        if (!!_pData->node)
        {
            GLTFCharValue json_value;
            if (!(_pData->node >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("node"), json_value, g_json_doc_ptr->GetAllocator());
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->path >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("path"), json_value, g_json_doc_ptr->GetAllocator());
        }
        return true;
    }

    bool operator<<(std::vector<std::shared_ptr<SAnimationChannelTarget>>& _vDatas, const GLTFCharValue& _JsonValue)
    {
        return operator<< <std::shared_ptr<SAnimationChannelTarget>>(_vDatas, _JsonValue);
    }

    bool operator>>(const std::vector<std::shared_ptr<SAnimationChannelTarget>>& _vDatas, GLTFCharValue& _JsonValue)
    {
        return operator>> <std::shared_ptr<SAnimationChannelTarget>>(_vDatas, _JsonValue);
    }

    bool operator<<(std::shared_ptr<SMesh>& _pData, const GLTFCharValue& _JsonValue)
    {
        std::shared_ptr<SMesh> data_ptr = !!_pData ? _pData : std::make_shared<SMesh>();
        {
            std::shared_ptr<SGlTFChildofRootProperty> super_ptr = data_ptr;
            if (!(super_ptr << _JsonValue)) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("primitives")) && _JsonValue[GLTFTEXT("primitives")].IsArray())
        {
            if (!(data_ptr->primitives << _JsonValue[GLTFTEXT("primitives")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("weights")) && _JsonValue[GLTFTEXT("weights")].IsArray())
        {
            if (!(data_ptr->weights << _JsonValue[GLTFTEXT("weights")])) return false;
        }
        _pData = data_ptr;
        return true;
    }

    bool operator>>(const std::shared_ptr<SMesh>& _pData, GLTFCharValue& _JsonValue)
    {
        if (!_pData || !g_json_doc_ptr) return false;
        _JsonValue.SetObject();
        {
            const std::shared_ptr<SGlTFChildofRootProperty> super_ptr = _pData;
            if (!(super_ptr >> _JsonValue)) return false;
        }
        if (!_pData->primitives.empty())
        {
            GLTFCharValue json_value;
            if (!(_pData->primitives >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("primitives"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!_pData->weights.empty())
        {
            GLTFCharValue json_value;
            if (!(_pData->weights >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("weights"), json_value, g_json_doc_ptr->GetAllocator());
        }
        return true;
    }

    bool operator<<(std::vector<std::shared_ptr<SMesh>>& _vDatas, const GLTFCharValue& _JsonValue)
    {
        return operator<< <std::shared_ptr<SMesh>>(_vDatas, _JsonValue);
    }

    bool operator>>(const std::vector<std::shared_ptr<SMesh>>& _vDatas, GLTFCharValue& _JsonValue)
    {
        return operator>> <std::shared_ptr<SMesh>>(_vDatas, _JsonValue);
    }

    bool operator<<(std::shared_ptr<SAccessorSparse>& _pData, const GLTFCharValue& _JsonValue)
    {
        std::shared_ptr<SAccessorSparse> data_ptr = !!_pData ? _pData : std::make_shared<SAccessorSparse>();
        {
            std::shared_ptr<SGlTFProperty> super_ptr = data_ptr;
            if (!(super_ptr << _JsonValue)) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("count")) && _JsonValue[GLTFTEXT("count")].IsInt())
        {
            data_ptr->count = _JsonValue[GLTFTEXT("count")].GetInt();
        }
        if (_JsonValue.HasMember(GLTFTEXT("indices")) && _JsonValue[GLTFTEXT("indices")].IsObject())
        {
            if (!(data_ptr->indices << _JsonValue[GLTFTEXT("indices")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("values")) && _JsonValue[GLTFTEXT("values")].IsObject())
        {
            if (!(data_ptr->values << _JsonValue[GLTFTEXT("values")])) return false;
        }
        _pData = data_ptr;
        return true;
    }

    bool operator>>(const std::shared_ptr<SAccessorSparse>& _pData, GLTFCharValue& _JsonValue)
    {
        if (!_pData || !g_json_doc_ptr) return false;
        _JsonValue.SetObject();
        {
            const std::shared_ptr<SGlTFProperty> super_ptr = _pData;
            if (!(super_ptr >> _JsonValue)) return false;
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->count >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("count"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!!_pData->indices)
        {
            GLTFCharValue json_value;
            if (!(_pData->indices >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("indices"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!!_pData->values)
        {
            GLTFCharValue json_value;
            if (!(_pData->values >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("values"), json_value, g_json_doc_ptr->GetAllocator());
        }
        return true;
    }

    bool operator<<(std::vector<std::shared_ptr<SAccessorSparse>>& _vDatas, const GLTFCharValue& _JsonValue)
    {
        return operator<< <std::shared_ptr<SAccessorSparse>>(_vDatas, _JsonValue);
    }

    bool operator>>(const std::vector<std::shared_ptr<SAccessorSparse>>& _vDatas, GLTFCharValue& _JsonValue)
    {
        return operator>> <std::shared_ptr<SAccessorSparse>>(_vDatas, _JsonValue);
    }

    bool operator<<(std::shared_ptr<SMeshPrimitive>& _pData, const GLTFCharValue& _JsonValue)
    {
        std::shared_ptr<SMeshPrimitive> data_ptr = !!_pData ? _pData : std::make_shared<SMeshPrimitive>();
        {
            std::shared_ptr<SGlTFProperty> super_ptr = data_ptr;
            if (!(super_ptr << _JsonValue)) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("indices")) && _JsonValue[GLTFTEXT("indices")].IsInt())
        {
            if (!(data_ptr->indices << _JsonValue[GLTFTEXT("indices")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("attributes")) && _JsonValue[GLTFTEXT("attributes")].IsObject())
        {
            if (!(data_ptr->attributes << _JsonValue[GLTFTEXT("attributes")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("material")) && _JsonValue[GLTFTEXT("material")].IsInt())
        {
            if (!(data_ptr->material << _JsonValue[GLTFTEXT("material")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("mode")) && _JsonValue[GLTFTEXT("mode")].IsInt())
        {
            data_ptr->mode = _JsonValue[GLTFTEXT("mode")].GetInt();
        }
        if (_JsonValue.HasMember(GLTFTEXT("targets")) && _JsonValue[GLTFTEXT("targets")].IsArray())
        {
            if (!(data_ptr->targets << _JsonValue[GLTFTEXT("targets")])) return false;
        }
        _pData = data_ptr;
        return true;
    }

    bool operator>>(const std::shared_ptr<SMeshPrimitive>& _pData, GLTFCharValue& _JsonValue)
    {
        if (!_pData || !g_json_doc_ptr) return false;
        _JsonValue.SetObject();
        {
            const std::shared_ptr<SGlTFProperty> super_ptr = _pData;
            if (!(super_ptr >> _JsonValue)) return false;
        }
        if (!!_pData->indices)
        {
            GLTFCharValue json_value;
            if (!(_pData->indices >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("indices"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!_pData->attributes.empty())
        {
            GLTFCharValue json_value;
            if (!(_pData->attributes >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("attributes"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!!_pData->material)
        {
            GLTFCharValue json_value;
            if (!(_pData->material >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("material"), json_value, g_json_doc_ptr->GetAllocator());
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->mode >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("mode"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!_pData->targets.empty())
        {
            GLTFCharValue json_value;
            if (!(_pData->targets >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("targets"), json_value, g_json_doc_ptr->GetAllocator());
        }
        return true;
    }

    bool operator<<(std::vector<std::shared_ptr<SMeshPrimitive>>& _vDatas, const GLTFCharValue& _JsonValue)
    {
        return operator<< <std::shared_ptr<SMeshPrimitive>>(_vDatas, _JsonValue);
    }

    bool operator>>(const std::vector<std::shared_ptr<SMeshPrimitive>>& _vDatas, GLTFCharValue& _JsonValue)
    {
        return operator>> <std::shared_ptr<SMeshPrimitive>>(_vDatas, _JsonValue);
    }

    bool operator<<(std::shared_ptr<SExtension>& _pData, const GLTFCharValue& _JsonValue)
    {
        std::shared_ptr<SExtension> data_ptr = !!_pData ? _pData : std::make_shared<SExtension>();
        _pData = data_ptr;
        return true;
    }

    bool operator>>(const std::shared_ptr<SExtension>& _pData, GLTFCharValue& _JsonValue)
    {
        if (!_pData || !g_json_doc_ptr) return false;
        _JsonValue.SetObject();
        return true;
    }

    bool operator<<(std::vector<std::shared_ptr<SExtension>>& _vDatas, const GLTFCharValue& _JsonValue)
    {
        return operator<< <std::shared_ptr<SExtension>>(_vDatas, _JsonValue);
    }

    bool operator>>(const std::vector<std::shared_ptr<SExtension>>& _vDatas, GLTFCharValue& _JsonValue)
    {
        return operator>> <std::shared_ptr<SExtension>>(_vDatas, _JsonValue);
    }

    bool operator<<(std::shared_ptr<SAnimationChannel>& _pData, const GLTFCharValue& _JsonValue)
    {
        std::shared_ptr<SAnimationChannel> data_ptr = !!_pData ? _pData : std::make_shared<SAnimationChannel>();
        {
            std::shared_ptr<SGlTFProperty> super_ptr = data_ptr;
            if (!(super_ptr << _JsonValue)) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("target")) && _JsonValue[GLTFTEXT("target")].IsObject())
        {
            if (!(data_ptr->target << _JsonValue[GLTFTEXT("target")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("sampler")) && _JsonValue[GLTFTEXT("sampler")].IsInt())
        {
            if (!(data_ptr->sampler << _JsonValue[GLTFTEXT("sampler")])) return false;
        }
        _pData = data_ptr;
        return true;
    }

    bool operator>>(const std::shared_ptr<SAnimationChannel>& _pData, GLTFCharValue& _JsonValue)
    {
        if (!_pData || !g_json_doc_ptr) return false;
        _JsonValue.SetObject();
        {
            const std::shared_ptr<SGlTFProperty> super_ptr = _pData;
            if (!(super_ptr >> _JsonValue)) return false;
        }
        if (!!_pData->target)
        {
            GLTFCharValue json_value;
            if (!(_pData->target >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("target"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!!_pData->sampler)
        {
            GLTFCharValue json_value;
            if (!(_pData->sampler >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("sampler"), json_value, g_json_doc_ptr->GetAllocator());
        }
        return true;
    }

    bool operator<<(std::vector<std::shared_ptr<SAnimationChannel>>& _vDatas, const GLTFCharValue& _JsonValue)
    {
        return operator<< <std::shared_ptr<SAnimationChannel>>(_vDatas, _JsonValue);
    }

    bool operator>>(const std::vector<std::shared_ptr<SAnimationChannel>>& _vDatas, GLTFCharValue& _JsonValue)
    {
        return operator>> <std::shared_ptr<SAnimationChannel>>(_vDatas, _JsonValue);
    }

    bool operator<<(std::shared_ptr<SGlTFId>& _pData, const GLTFCharValue& _JsonValue)
    {
        std::shared_ptr<SGlTFId> data_ptr = !!_pData ? _pData : std::make_shared<SGlTFId>();
        if (!(data_ptr->int32_tValue << _JsonValue)) return false;
        _pData = data_ptr;
        return true;
    }

    bool operator>>(const std::shared_ptr<SGlTFId>& _pData, GLTFCharValue& _JsonValue)
    {
        if (!_pData || !g_json_doc_ptr) return false;
        _JsonValue.SetObject();
        if (!(_pData->int32_tValue >> _JsonValue)) return false;
        return true;
    }

    bool operator<<(std::vector<std::shared_ptr<SGlTFId>>& _vDatas, const GLTFCharValue& _JsonValue)
    {
        return operator<< <std::shared_ptr<SGlTFId>>(_vDatas, _JsonValue);
    }

    bool operator>>(const std::vector<std::shared_ptr<SGlTFId>>& _vDatas, GLTFCharValue& _JsonValue)
    {
        return operator>> <std::shared_ptr<SGlTFId>>(_vDatas, _JsonValue);
    }

    bool operator<<(std::shared_ptr<SAccessorSparseIndices>& _pData, const GLTFCharValue& _JsonValue)
    {
        std::shared_ptr<SAccessorSparseIndices> data_ptr = !!_pData ? _pData : std::make_shared<SAccessorSparseIndices>();
        {
            std::shared_ptr<SGlTFProperty> super_ptr = data_ptr;
            if (!(super_ptr << _JsonValue)) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("componentType")) && _JsonValue[GLTFTEXT("componentType")].IsInt())
        {
            data_ptr->componentType = _JsonValue[GLTFTEXT("componentType")].GetInt();
        }
        if (_JsonValue.HasMember(GLTFTEXT("bufferView")) && _JsonValue[GLTFTEXT("bufferView")].IsInt())
        {
            if (!(data_ptr->bufferView << _JsonValue[GLTFTEXT("bufferView")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("byteOffset")) && _JsonValue[GLTFTEXT("byteOffset")].IsInt())
        {
            data_ptr->byteOffset = _JsonValue[GLTFTEXT("byteOffset")].GetInt();
        }
        _pData = data_ptr;
        return true;
    }

    bool operator>>(const std::shared_ptr<SAccessorSparseIndices>& _pData, GLTFCharValue& _JsonValue)
    {
        if (!_pData || !g_json_doc_ptr) return false;
        _JsonValue.SetObject();
        {
            const std::shared_ptr<SGlTFProperty> super_ptr = _pData;
            if (!(super_ptr >> _JsonValue)) return false;
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->componentType >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("componentType"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!!_pData->bufferView)
        {
            GLTFCharValue json_value;
            if (!(_pData->bufferView >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("bufferView"), json_value, g_json_doc_ptr->GetAllocator());
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->byteOffset >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("byteOffset"), json_value, g_json_doc_ptr->GetAllocator());
        }
        return true;
    }

    bool operator<<(std::vector<std::shared_ptr<SAccessorSparseIndices>>& _vDatas, const GLTFCharValue& _JsonValue)
    {
        return operator<< <std::shared_ptr<SAccessorSparseIndices>>(_vDatas, _JsonValue);
    }

    bool operator>>(const std::vector<std::shared_ptr<SAccessorSparseIndices>>& _vDatas, GLTFCharValue& _JsonValue)
    {
        return operator>> <std::shared_ptr<SAccessorSparseIndices>>(_vDatas, _JsonValue);
    }

    bool operator<<(std::shared_ptr<SNode>& _pData, const GLTFCharValue& _JsonValue)
    {
        std::shared_ptr<SNode> data_ptr = !!_pData ? _pData : std::make_shared<SNode>();
        {
            std::shared_ptr<SGlTFChildofRootProperty> super_ptr = data_ptr;
            if (!(super_ptr << _JsonValue)) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("scale")) && _JsonValue[GLTFTEXT("scale")].IsArray())
        {
            if (!(data_ptr->scale << _JsonValue[GLTFTEXT("scale")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("rotation")) && _JsonValue[GLTFTEXT("rotation")].IsArray())
        {
            if (!(data_ptr->rotation << _JsonValue[GLTFTEXT("rotation")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("matrix")) && _JsonValue[GLTFTEXT("matrix")].IsArray())
        {
            if (!(data_ptr->matrix << _JsonValue[GLTFTEXT("matrix")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("mesh")) && _JsonValue[GLTFTEXT("mesh")].IsInt())
        {
            if (!(data_ptr->mesh << _JsonValue[GLTFTEXT("mesh")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("camera")) && _JsonValue[GLTFTEXT("camera")].IsInt())
        {
            if (!(data_ptr->camera << _JsonValue[GLTFTEXT("camera")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("weights")) && _JsonValue[GLTFTEXT("weights")].IsArray())
        {
            if (!(data_ptr->weights << _JsonValue[GLTFTEXT("weights")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("skin")) && _JsonValue[GLTFTEXT("skin")].IsInt())
        {
            if (!(data_ptr->skin << _JsonValue[GLTFTEXT("skin")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("translation")) && _JsonValue[GLTFTEXT("translation")].IsArray())
        {
            if (!(data_ptr->translation << _JsonValue[GLTFTEXT("translation")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("children")) && _JsonValue[GLTFTEXT("children")].IsArray())
        {
            if (!(data_ptr->children << _JsonValue[GLTFTEXT("children")])) return false;
        }
        _pData = data_ptr;
        return true;
    }

    bool operator>>(const std::shared_ptr<SNode>& _pData, GLTFCharValue& _JsonValue)
    {
        if (!_pData || !g_json_doc_ptr) return false;
        _JsonValue.SetObject();
        {
            const std::shared_ptr<SGlTFChildofRootProperty> super_ptr = _pData;
            if (!(super_ptr >> _JsonValue)) return false;
        }
        if (!_pData->scale.empty())
        {
            GLTFCharValue json_value;
            if (!(_pData->scale >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("scale"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!_pData->rotation.empty())
        {
            GLTFCharValue json_value;
            if (!(_pData->rotation >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("rotation"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!_pData->matrix.empty())
        {
            GLTFCharValue json_value;
            if (!(_pData->matrix >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("matrix"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!!_pData->mesh)
        {
            GLTFCharValue json_value;
            if (!(_pData->mesh >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("mesh"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!!_pData->camera)
        {
            GLTFCharValue json_value;
            if (!(_pData->camera >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("camera"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!_pData->weights.empty())
        {
            GLTFCharValue json_value;
            if (!(_pData->weights >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("weights"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!!_pData->skin)
        {
            GLTFCharValue json_value;
            if (!(_pData->skin >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("skin"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!_pData->translation.empty())
        {
            GLTFCharValue json_value;
            if (!(_pData->translation >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("translation"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!_pData->children.empty())
        {
            GLTFCharValue json_value;
            if (!(_pData->children >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("children"), json_value, g_json_doc_ptr->GetAllocator());
        }
        return true;
    }

    bool operator<<(std::vector<std::shared_ptr<SNode>>& _vDatas, const GLTFCharValue& _JsonValue)
    {
        return operator<< <std::shared_ptr<SNode>>(_vDatas, _JsonValue);
    }

    bool operator>>(const std::vector<std::shared_ptr<SNode>>& _vDatas, GLTFCharValue& _JsonValue)
    {
        return operator>> <std::shared_ptr<SNode>>(_vDatas, _JsonValue);
    }

    bool operator<<(std::shared_ptr<SAnimation>& _pData, const GLTFCharValue& _JsonValue)
    {
        std::shared_ptr<SAnimation> data_ptr = !!_pData ? _pData : std::make_shared<SAnimation>();
        {
            std::shared_ptr<SGlTFChildofRootProperty> super_ptr = data_ptr;
            if (!(super_ptr << _JsonValue)) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("channels")) && _JsonValue[GLTFTEXT("channels")].IsArray())
        {
            if (!(data_ptr->channels << _JsonValue[GLTFTEXT("channels")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("samplers")) && _JsonValue[GLTFTEXT("samplers")].IsArray())
        {
            if (!(data_ptr->samplers << _JsonValue[GLTFTEXT("samplers")])) return false;
        }
        _pData = data_ptr;
        return true;
    }

    bool operator>>(const std::shared_ptr<SAnimation>& _pData, GLTFCharValue& _JsonValue)
    {
        if (!_pData || !g_json_doc_ptr) return false;
        _JsonValue.SetObject();
        {
            const std::shared_ptr<SGlTFChildofRootProperty> super_ptr = _pData;
            if (!(super_ptr >> _JsonValue)) return false;
        }
        if (!_pData->channels.empty())
        {
            GLTFCharValue json_value;
            if (!(_pData->channels >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("channels"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!_pData->samplers.empty())
        {
            GLTFCharValue json_value;
            if (!(_pData->samplers >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("samplers"), json_value, g_json_doc_ptr->GetAllocator());
        }
        return true;
    }

    bool operator<<(std::vector<std::shared_ptr<SAnimation>>& _vDatas, const GLTFCharValue& _JsonValue)
    {
        return operator<< <std::shared_ptr<SAnimation>>(_vDatas, _JsonValue);
    }

    bool operator>>(const std::vector<std::shared_ptr<SAnimation>>& _vDatas, GLTFCharValue& _JsonValue)
    {
        return operator>> <std::shared_ptr<SAnimation>>(_vDatas, _JsonValue);
    }

    bool operator<<(std::shared_ptr<SSkin>& _pData, const GLTFCharValue& _JsonValue)
    {
        std::shared_ptr<SSkin> data_ptr = !!_pData ? _pData : std::make_shared<SSkin>();
        {
            std::shared_ptr<SGlTFChildofRootProperty> super_ptr = data_ptr;
            if (!(super_ptr << _JsonValue)) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("joints")) && _JsonValue[GLTFTEXT("joints")].IsArray())
        {
            if (!(data_ptr->joints << _JsonValue[GLTFTEXT("joints")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("inverseBindMatrices")) && _JsonValue[GLTFTEXT("inverseBindMatrices")].IsInt())
        {
            if (!(data_ptr->inverseBindMatrices << _JsonValue[GLTFTEXT("inverseBindMatrices")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("skeleton")) && _JsonValue[GLTFTEXT("skeleton")].IsInt())
        {
            if (!(data_ptr->skeleton << _JsonValue[GLTFTEXT("skeleton")])) return false;
        }
        _pData = data_ptr;
        return true;
    }

    bool operator>>(const std::shared_ptr<SSkin>& _pData, GLTFCharValue& _JsonValue)
    {
        if (!_pData || !g_json_doc_ptr) return false;
        _JsonValue.SetObject();
        {
            const std::shared_ptr<SGlTFChildofRootProperty> super_ptr = _pData;
            if (!(super_ptr >> _JsonValue)) return false;
        }
        if (!_pData->joints.empty())
        {
            GLTFCharValue json_value;
            if (!(_pData->joints >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("joints"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!!_pData->inverseBindMatrices)
        {
            GLTFCharValue json_value;
            if (!(_pData->inverseBindMatrices >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("inverseBindMatrices"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!!_pData->skeleton)
        {
            GLTFCharValue json_value;
            if (!(_pData->skeleton >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("skeleton"), json_value, g_json_doc_ptr->GetAllocator());
        }
        return true;
    }

    bool operator<<(std::vector<std::shared_ptr<SSkin>>& _vDatas, const GLTFCharValue& _JsonValue)
    {
        return operator<< <std::shared_ptr<SSkin>>(_vDatas, _JsonValue);
    }

    bool operator>>(const std::vector<std::shared_ptr<SSkin>>& _vDatas, GLTFCharValue& _JsonValue)
    {
        return operator>> <std::shared_ptr<SSkin>>(_vDatas, _JsonValue);
    }

    bool operator<<(std::shared_ptr<SMaterialPBRMetallicRoughness>& _pData, const GLTFCharValue& _JsonValue)
    {
        std::shared_ptr<SMaterialPBRMetallicRoughness> data_ptr = !!_pData ? _pData : std::make_shared<SMaterialPBRMetallicRoughness>();
        {
            std::shared_ptr<SGlTFProperty> super_ptr = data_ptr;
            if (!(super_ptr << _JsonValue)) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("roughnessFactor")) && _JsonValue[GLTFTEXT("roughnessFactor")].IsFloat())
        {
            data_ptr->roughnessFactor = _JsonValue[GLTFTEXT("roughnessFactor")].GetFloat();
        }
        if (_JsonValue.HasMember(GLTFTEXT("baseColorTexture")) && _JsonValue[GLTFTEXT("baseColorTexture")].IsObject())
        {
            if (!(data_ptr->baseColorTexture << _JsonValue[GLTFTEXT("baseColorTexture")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("metallicFactor")) && _JsonValue[GLTFTEXT("metallicFactor")].IsFloat())
        {
            data_ptr->metallicFactor = _JsonValue[GLTFTEXT("metallicFactor")].GetFloat();
        }
        if (_JsonValue.HasMember(GLTFTEXT("baseColorFactor")) && _JsonValue[GLTFTEXT("baseColorFactor")].IsArray())
        {
            if (!(data_ptr->baseColorFactor << _JsonValue[GLTFTEXT("baseColorFactor")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("metallicRoughnessTexture")) && _JsonValue[GLTFTEXT("metallicRoughnessTexture")].IsObject())
        {
            if (!(data_ptr->metallicRoughnessTexture << _JsonValue[GLTFTEXT("metallicRoughnessTexture")])) return false;
        }
        _pData = data_ptr;
        return true;
    }

    bool operator>>(const std::shared_ptr<SMaterialPBRMetallicRoughness>& _pData, GLTFCharValue& _JsonValue)
    {
        if (!_pData || !g_json_doc_ptr) return false;
        _JsonValue.SetObject();
        {
            const std::shared_ptr<SGlTFProperty> super_ptr = _pData;
            if (!(super_ptr >> _JsonValue)) return false;
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->roughnessFactor >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("roughnessFactor"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!!_pData->baseColorTexture)
        {
            GLTFCharValue json_value;
            if (!(_pData->baseColorTexture >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("baseColorTexture"), json_value, g_json_doc_ptr->GetAllocator());
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->metallicFactor >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("metallicFactor"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!_pData->baseColorFactor.empty())
        {
            GLTFCharValue json_value;
            if (!(_pData->baseColorFactor >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("baseColorFactor"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!!_pData->metallicRoughnessTexture)
        {
            GLTFCharValue json_value;
            if (!(_pData->metallicRoughnessTexture >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("metallicRoughnessTexture"), json_value, g_json_doc_ptr->GetAllocator());
        }
        return true;
    }

    bool operator<<(std::vector<std::shared_ptr<SMaterialPBRMetallicRoughness>>& _vDatas, const GLTFCharValue& _JsonValue)
    {
        return operator<< <std::shared_ptr<SMaterialPBRMetallicRoughness>>(_vDatas, _JsonValue);
    }

    bool operator>>(const std::vector<std::shared_ptr<SMaterialPBRMetallicRoughness>>& _vDatas, GLTFCharValue& _JsonValue)
    {
        return operator>> <std::shared_ptr<SMaterialPBRMetallicRoughness>>(_vDatas, _JsonValue);
    }

    bool operator<<(std::shared_ptr<SCamera>& _pData, const GLTFCharValue& _JsonValue)
    {
        std::shared_ptr<SCamera> data_ptr = !!_pData ? _pData : std::make_shared<SCamera>();
        {
            std::shared_ptr<SGlTFChildofRootProperty> super_ptr = data_ptr;
            if (!(super_ptr << _JsonValue)) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("type")) && _JsonValue[GLTFTEXT("type")].IsString())
        {
            data_ptr->type = _JsonValue[GLTFTEXT("type")].GetString();
        }
        if (_JsonValue.HasMember(GLTFTEXT("perspective")) && _JsonValue[GLTFTEXT("perspective")].IsObject())
        {
            if (!(data_ptr->perspective << _JsonValue[GLTFTEXT("perspective")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("orthographic")) && _JsonValue[GLTFTEXT("orthographic")].IsObject())
        {
            if (!(data_ptr->orthographic << _JsonValue[GLTFTEXT("orthographic")])) return false;
        }
        _pData = data_ptr;
        return true;
    }

    bool operator>>(const std::shared_ptr<SCamera>& _pData, GLTFCharValue& _JsonValue)
    {
        if (!_pData || !g_json_doc_ptr) return false;
        _JsonValue.SetObject();
        {
            const std::shared_ptr<SGlTFChildofRootProperty> super_ptr = _pData;
            if (!(super_ptr >> _JsonValue)) return false;
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->type >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("type"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!!_pData->perspective)
        {
            GLTFCharValue json_value;
            if (!(_pData->perspective >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("perspective"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!!_pData->orthographic)
        {
            GLTFCharValue json_value;
            if (!(_pData->orthographic >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("orthographic"), json_value, g_json_doc_ptr->GetAllocator());
        }
        return true;
    }

    bool operator<<(std::vector<std::shared_ptr<SCamera>>& _vDatas, const GLTFCharValue& _JsonValue)
    {
        return operator<< <std::shared_ptr<SCamera>>(_vDatas, _JsonValue);
    }

    bool operator>>(const std::vector<std::shared_ptr<SCamera>>& _vDatas, GLTFCharValue& _JsonValue)
    {
        return operator>> <std::shared_ptr<SCamera>>(_vDatas, _JsonValue);
    }

    bool operator<<(std::shared_ptr<SImage>& _pData, const GLTFCharValue& _JsonValue)
    {
        std::shared_ptr<SImage> data_ptr = !!_pData ? _pData : std::make_shared<SImage>();
        {
            std::shared_ptr<SGlTFChildofRootProperty> super_ptr = data_ptr;
            if (!(super_ptr << _JsonValue)) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("mimeType")) && _JsonValue[GLTFTEXT("mimeType")].IsString())
        {
            data_ptr->mimeType = _JsonValue[GLTFTEXT("mimeType")].GetString();
        }
        if (_JsonValue.HasMember(GLTFTEXT("bufferView")) && _JsonValue[GLTFTEXT("bufferView")].IsInt())
        {
            if (!(data_ptr->bufferView << _JsonValue[GLTFTEXT("bufferView")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("uri")) && _JsonValue[GLTFTEXT("uri")].IsString())
        {
            data_ptr->uri = _JsonValue[GLTFTEXT("uri")].GetString();
        }
        _pData = data_ptr;
        return true;
    }

    bool operator>>(const std::shared_ptr<SImage>& _pData, GLTFCharValue& _JsonValue)
    {
        if (!_pData || !g_json_doc_ptr) return false;
        _JsonValue.SetObject();
        {
            const std::shared_ptr<SGlTFChildofRootProperty> super_ptr = _pData;
            if (!(super_ptr >> _JsonValue)) return false;
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->mimeType >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("mimeType"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!!_pData->bufferView)
        {
            GLTFCharValue json_value;
            if (!(_pData->bufferView >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("bufferView"), json_value, g_json_doc_ptr->GetAllocator());
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->uri >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("uri"), json_value, g_json_doc_ptr->GetAllocator());
        }
        return true;
    }

    bool operator<<(std::vector<std::shared_ptr<SImage>>& _vDatas, const GLTFCharValue& _JsonValue)
    {
        return operator<< <std::shared_ptr<SImage>>(_vDatas, _JsonValue);
    }

    bool operator>>(const std::vector<std::shared_ptr<SImage>>& _vDatas, GLTFCharValue& _JsonValue)
    {
        return operator>> <std::shared_ptr<SImage>>(_vDatas, _JsonValue);
    }

    bool operator<<(std::shared_ptr<STexture>& _pData, const GLTFCharValue& _JsonValue)
    {
        std::shared_ptr<STexture> data_ptr = !!_pData ? _pData : std::make_shared<STexture>();
        {
            std::shared_ptr<SGlTFChildofRootProperty> super_ptr = data_ptr;
            if (!(super_ptr << _JsonValue)) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("source")) && _JsonValue[GLTFTEXT("source")].IsInt())
        {
            if (!(data_ptr->source << _JsonValue[GLTFTEXT("source")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("sampler")) && _JsonValue[GLTFTEXT("sampler")].IsInt())
        {
            if (!(data_ptr->sampler << _JsonValue[GLTFTEXT("sampler")])) return false;
        }
        _pData = data_ptr;
        return true;
    }

    bool operator>>(const std::shared_ptr<STexture>& _pData, GLTFCharValue& _JsonValue)
    {
        if (!_pData || !g_json_doc_ptr) return false;
        _JsonValue.SetObject();
        {
            const std::shared_ptr<SGlTFChildofRootProperty> super_ptr = _pData;
            if (!(super_ptr >> _JsonValue)) return false;
        }
        if (!!_pData->source)
        {
            GLTFCharValue json_value;
            if (!(_pData->source >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("source"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!!_pData->sampler)
        {
            GLTFCharValue json_value;
            if (!(_pData->sampler >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("sampler"), json_value, g_json_doc_ptr->GetAllocator());
        }
        return true;
    }

    bool operator<<(std::vector<std::shared_ptr<STexture>>& _vDatas, const GLTFCharValue& _JsonValue)
    {
        return operator<< <std::shared_ptr<STexture>>(_vDatas, _JsonValue);
    }

    bool operator>>(const std::vector<std::shared_ptr<STexture>>& _vDatas, GLTFCharValue& _JsonValue)
    {
        return operator>> <std::shared_ptr<STexture>>(_vDatas, _JsonValue);
    }

    bool operator<<(std::shared_ptr<SCameraOrthographic>& _pData, const GLTFCharValue& _JsonValue)
    {
        std::shared_ptr<SCameraOrthographic> data_ptr = !!_pData ? _pData : std::make_shared<SCameraOrthographic>();
        {
            std::shared_ptr<SGlTFProperty> super_ptr = data_ptr;
            if (!(super_ptr << _JsonValue)) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("xmag")) && _JsonValue[GLTFTEXT("xmag")].IsFloat())
        {
            data_ptr->xmag = _JsonValue[GLTFTEXT("xmag")].GetFloat();
        }
        if (_JsonValue.HasMember(GLTFTEXT("ymag")) && _JsonValue[GLTFTEXT("ymag")].IsFloat())
        {
            data_ptr->ymag = _JsonValue[GLTFTEXT("ymag")].GetFloat();
        }
        if (_JsonValue.HasMember(GLTFTEXT("zfar")) && _JsonValue[GLTFTEXT("zfar")].IsFloat())
        {
            data_ptr->zfar = _JsonValue[GLTFTEXT("zfar")].GetFloat();
        }
        if (_JsonValue.HasMember(GLTFTEXT("znear")) && _JsonValue[GLTFTEXT("znear")].IsFloat())
        {
            data_ptr->znear = _JsonValue[GLTFTEXT("znear")].GetFloat();
        }
        _pData = data_ptr;
        return true;
    }

    bool operator>>(const std::shared_ptr<SCameraOrthographic>& _pData, GLTFCharValue& _JsonValue)
    {
        if (!_pData || !g_json_doc_ptr) return false;
        _JsonValue.SetObject();
        {
            const std::shared_ptr<SGlTFProperty> super_ptr = _pData;
            if (!(super_ptr >> _JsonValue)) return false;
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->xmag >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("xmag"), json_value, g_json_doc_ptr->GetAllocator());
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->ymag >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("ymag"), json_value, g_json_doc_ptr->GetAllocator());
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->zfar >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("zfar"), json_value, g_json_doc_ptr->GetAllocator());
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->znear >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("znear"), json_value, g_json_doc_ptr->GetAllocator());
        }
        return true;
    }

    bool operator<<(std::vector<std::shared_ptr<SCameraOrthographic>>& _vDatas, const GLTFCharValue& _JsonValue)
    {
        return operator<< <std::shared_ptr<SCameraOrthographic>>(_vDatas, _JsonValue);
    }

    bool operator>>(const std::vector<std::shared_ptr<SCameraOrthographic>>& _vDatas, GLTFCharValue& _JsonValue)
    {
        return operator>> <std::shared_ptr<SCameraOrthographic>>(_vDatas, _JsonValue);
    }

    bool operator<<(std::shared_ptr<SBuffer>& _pData, const GLTFCharValue& _JsonValue)
    {
        std::shared_ptr<SBuffer> data_ptr = !!_pData ? _pData : std::make_shared<SBuffer>();
        {
            std::shared_ptr<SGlTFChildofRootProperty> super_ptr = data_ptr;
            if (!(super_ptr << _JsonValue)) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("byteLength")) && _JsonValue[GLTFTEXT("byteLength")].IsInt())
        {
            data_ptr->byteLength = _JsonValue[GLTFTEXT("byteLength")].GetInt();
        }
        if (_JsonValue.HasMember(GLTFTEXT("uri")) && _JsonValue[GLTFTEXT("uri")].IsString())
        {
            data_ptr->uri = _JsonValue[GLTFTEXT("uri")].GetString();
        }
        _pData = data_ptr;
        return true;
    }

    bool operator>>(const std::shared_ptr<SBuffer>& _pData, GLTFCharValue& _JsonValue)
    {
        if (!_pData || !g_json_doc_ptr) return false;
        _JsonValue.SetObject();
        {
            const std::shared_ptr<SGlTFChildofRootProperty> super_ptr = _pData;
            if (!(super_ptr >> _JsonValue)) return false;
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->byteLength >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("byteLength"), json_value, g_json_doc_ptr->GetAllocator());
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->uri >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("uri"), json_value, g_json_doc_ptr->GetAllocator());
        }
        return true;
    }

    bool operator<<(std::vector<std::shared_ptr<SBuffer>>& _vDatas, const GLTFCharValue& _JsonValue)
    {
        return operator<< <std::shared_ptr<SBuffer>>(_vDatas, _JsonValue);
    }

    bool operator>>(const std::vector<std::shared_ptr<SBuffer>>& _vDatas, GLTFCharValue& _JsonValue)
    {
        return operator>> <std::shared_ptr<SBuffer>>(_vDatas, _JsonValue);
    }

    bool operator<<(std::shared_ptr<SAccessor>& _pData, const GLTFCharValue& _JsonValue)
    {
        std::shared_ptr<SAccessor> data_ptr = !!_pData ? _pData : std::make_shared<SAccessor>();
        {
            std::shared_ptr<SGlTFChildofRootProperty> super_ptr = data_ptr;
            if (!(super_ptr << _JsonValue)) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("count")) && _JsonValue[GLTFTEXT("count")].IsInt())
        {
            data_ptr->count = _JsonValue[GLTFTEXT("count")].GetInt();
        }
        if (_JsonValue.HasMember(GLTFTEXT("min")) && _JsonValue[GLTFTEXT("min")].IsArray())
        {
            if (!(data_ptr->min << _JsonValue[GLTFTEXT("min")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("max")) && _JsonValue[GLTFTEXT("max")].IsArray())
        {
            if (!(data_ptr->max << _JsonValue[GLTFTEXT("max")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("bufferView")) && _JsonValue[GLTFTEXT("bufferView")].IsInt())
        {
            if (!(data_ptr->bufferView << _JsonValue[GLTFTEXT("bufferView")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("componentType")) && _JsonValue[GLTFTEXT("componentType")].IsInt())
        {
            data_ptr->componentType = _JsonValue[GLTFTEXT("componentType")].GetInt();
        }
        if (_JsonValue.HasMember(GLTFTEXT("byteOffset")) && _JsonValue[GLTFTEXT("byteOffset")].IsInt())
        {
            data_ptr->byteOffset = _JsonValue[GLTFTEXT("byteOffset")].GetInt();
        }
        if (_JsonValue.HasMember(GLTFTEXT("sparse")) && _JsonValue[GLTFTEXT("sparse")].IsObject())
        {
            if (!(data_ptr->sparse << _JsonValue[GLTFTEXT("sparse")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("type")) && _JsonValue[GLTFTEXT("type")].IsString())
        {
            data_ptr->type = _JsonValue[GLTFTEXT("type")].GetString();
        }
        if (_JsonValue.HasMember(GLTFTEXT("normalized")) && _JsonValue[GLTFTEXT("normalized")].IsBool())
        {
            data_ptr->normalized = _JsonValue[GLTFTEXT("normalized")].GetBool();
        }
        _pData = data_ptr;
        return true;
    }

    bool operator>>(const std::shared_ptr<SAccessor>& _pData, GLTFCharValue& _JsonValue)
    {
        if (!_pData || !g_json_doc_ptr) return false;
        _JsonValue.SetObject();
        {
            const std::shared_ptr<SGlTFChildofRootProperty> super_ptr = _pData;
            if (!(super_ptr >> _JsonValue)) return false;
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->count >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("count"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!_pData->min.empty())
        {
            GLTFCharValue json_value;
            if (!(_pData->min >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("min"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!_pData->max.empty())
        {
            GLTFCharValue json_value;
            if (!(_pData->max >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("max"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!!_pData->bufferView)
        {
            GLTFCharValue json_value;
            if (!(_pData->bufferView >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("bufferView"), json_value, g_json_doc_ptr->GetAllocator());
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->componentType >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("componentType"), json_value, g_json_doc_ptr->GetAllocator());
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->byteOffset >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("byteOffset"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!!_pData->sparse)
        {
            GLTFCharValue json_value;
            if (!(_pData->sparse >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("sparse"), json_value, g_json_doc_ptr->GetAllocator());
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->type >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("type"), json_value, g_json_doc_ptr->GetAllocator());
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->normalized >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("normalized"), json_value, g_json_doc_ptr->GetAllocator());
        }
        return true;
    }

    bool operator<<(std::vector<std::shared_ptr<SAccessor>>& _vDatas, const GLTFCharValue& _JsonValue)
    {
        return operator<< <std::shared_ptr<SAccessor>>(_vDatas, _JsonValue);
    }

    bool operator>>(const std::vector<std::shared_ptr<SAccessor>>& _vDatas, GLTFCharValue& _JsonValue)
    {
        return operator>> <std::shared_ptr<SAccessor>>(_vDatas, _JsonValue);
    }

    bool operator<<(std::shared_ptr<SGlTF>& _pData, const GLTFCharValue& _JsonValue)
    {
        std::shared_ptr<SGlTF> data_ptr = !!_pData ? _pData : std::make_shared<SGlTF>();
        {
            std::shared_ptr<SGlTFProperty> super_ptr = data_ptr;
            if (!(super_ptr << _JsonValue)) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("textures")) && _JsonValue[GLTFTEXT("textures")].IsArray())
        {
            if (!(data_ptr->textures << _JsonValue[GLTFTEXT("textures")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("cameras")) && _JsonValue[GLTFTEXT("cameras")].IsArray())
        {
            if (!(data_ptr->cameras << _JsonValue[GLTFTEXT("cameras")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("accessors")) && _JsonValue[GLTFTEXT("accessors")].IsArray())
        {
            if (!(data_ptr->accessors << _JsonValue[GLTFTEXT("accessors")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("extensionsUsed")) && _JsonValue[GLTFTEXT("extensionsUsed")].IsArray())
        {
            if (!(data_ptr->extensionsUsed << _JsonValue[GLTFTEXT("extensionsUsed")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("samplers")) && _JsonValue[GLTFTEXT("samplers")].IsArray())
        {
            if (!(data_ptr->samplers << _JsonValue[GLTFTEXT("samplers")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("scenes")) && _JsonValue[GLTFTEXT("scenes")].IsArray())
        {
            if (!(data_ptr->scenes << _JsonValue[GLTFTEXT("scenes")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("scene")) && _JsonValue[GLTFTEXT("scene")].IsInt())
        {
            if (!(data_ptr->scene << _JsonValue[GLTFTEXT("scene")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("extensionsRequired")) && _JsonValue[GLTFTEXT("extensionsRequired")].IsArray())
        {
            if (!(data_ptr->extensionsRequired << _JsonValue[GLTFTEXT("extensionsRequired")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("meshes")) && _JsonValue[GLTFTEXT("meshes")].IsArray())
        {
            if (!(data_ptr->meshes << _JsonValue[GLTFTEXT("meshes")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("animations")) && _JsonValue[GLTFTEXT("animations")].IsArray())
        {
            if (!(data_ptr->animations << _JsonValue[GLTFTEXT("animations")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("images")) && _JsonValue[GLTFTEXT("images")].IsArray())
        {
            if (!(data_ptr->images << _JsonValue[GLTFTEXT("images")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("nodes")) && _JsonValue[GLTFTEXT("nodes")].IsArray())
        {
            if (!(data_ptr->nodes << _JsonValue[GLTFTEXT("nodes")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("bufferViews")) && _JsonValue[GLTFTEXT("bufferViews")].IsArray())
        {
            if (!(data_ptr->bufferViews << _JsonValue[GLTFTEXT("bufferViews")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("skins")) && _JsonValue[GLTFTEXT("skins")].IsArray())
        {
            if (!(data_ptr->skins << _JsonValue[GLTFTEXT("skins")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("materials")) && _JsonValue[GLTFTEXT("materials")].IsArray())
        {
            if (!(data_ptr->materials << _JsonValue[GLTFTEXT("materials")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("buffers")) && _JsonValue[GLTFTEXT("buffers")].IsArray())
        {
            if (!(data_ptr->buffers << _JsonValue[GLTFTEXT("buffers")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("asset")) && _JsonValue[GLTFTEXT("asset")].IsObject())
        {
            if (!(data_ptr->asset << _JsonValue[GLTFTEXT("asset")])) return false;
        }
        _pData = data_ptr;
        return true;
    }

    bool operator>>(const std::shared_ptr<SGlTF>& _pData, GLTFCharValue& _JsonValue)
    {
        if (!_pData || !g_json_doc_ptr) return false;
        _JsonValue.SetObject();
        {
            const std::shared_ptr<SGlTFProperty> super_ptr = _pData;
            if (!(super_ptr >> _JsonValue)) return false;
        }
        if (!_pData->textures.empty())
        {
            GLTFCharValue json_value;
            if (!(_pData->textures >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("textures"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!_pData->cameras.empty())
        {
            GLTFCharValue json_value;
            if (!(_pData->cameras >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("cameras"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!_pData->accessors.empty())
        {
            GLTFCharValue json_value;
            if (!(_pData->accessors >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("accessors"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!_pData->extensionsUsed.empty())
        {
            GLTFCharValue json_value;
            if (!(_pData->extensionsUsed >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("extensionsUsed"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!_pData->samplers.empty())
        {
            GLTFCharValue json_value;
            if (!(_pData->samplers >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("samplers"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!_pData->scenes.empty())
        {
            GLTFCharValue json_value;
            if (!(_pData->scenes >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("scenes"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!!_pData->scene)
        {
            GLTFCharValue json_value;
            if (!(_pData->scene >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("scene"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!_pData->extensionsRequired.empty())
        {
            GLTFCharValue json_value;
            if (!(_pData->extensionsRequired >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("extensionsRequired"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!_pData->meshes.empty())
        {
            GLTFCharValue json_value;
            if (!(_pData->meshes >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("meshes"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!_pData->animations.empty())
        {
            GLTFCharValue json_value;
            if (!(_pData->animations >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("animations"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!_pData->images.empty())
        {
            GLTFCharValue json_value;
            if (!(_pData->images >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("images"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!_pData->nodes.empty())
        {
            GLTFCharValue json_value;
            if (!(_pData->nodes >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("nodes"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!_pData->bufferViews.empty())
        {
            GLTFCharValue json_value;
            if (!(_pData->bufferViews >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("bufferViews"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!_pData->skins.empty())
        {
            GLTFCharValue json_value;
            if (!(_pData->skins >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("skins"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!_pData->materials.empty())
        {
            GLTFCharValue json_value;
            if (!(_pData->materials >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("materials"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!_pData->buffers.empty())
        {
            GLTFCharValue json_value;
            if (!(_pData->buffers >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("buffers"), json_value, g_json_doc_ptr->GetAllocator());
        }
        if (!!_pData->asset)
        {
            GLTFCharValue json_value;
            if (!(_pData->asset >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("asset"), json_value, g_json_doc_ptr->GetAllocator());
        }
        return true;
    }

    bool operator<<(std::vector<std::shared_ptr<SGlTF>>& _vDatas, const GLTFCharValue& _JsonValue)
    {
        return operator<< <std::shared_ptr<SGlTF>>(_vDatas, _JsonValue);
    }

    bool operator>>(const std::vector<std::shared_ptr<SGlTF>>& _vDatas, GLTFCharValue& _JsonValue)
    {
        return operator>> <std::shared_ptr<SGlTF>>(_vDatas, _JsonValue);
    }

    bool operator<<(std::shared_ptr<STextureInfo>& _pData, const GLTFCharValue& _JsonValue)
    {
        std::shared_ptr<STextureInfo> data_ptr = !!_pData ? _pData : std::make_shared<STextureInfo>();
        {
            std::shared_ptr<SGlTFProperty> super_ptr = data_ptr;
            if (!(super_ptr << _JsonValue)) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("index")) && _JsonValue[GLTFTEXT("index")].IsInt())
        {
            if (!(data_ptr->index << _JsonValue[GLTFTEXT("index")])) return false;
        }
        if (_JsonValue.HasMember(GLTFTEXT("texCoord")) && _JsonValue[GLTFTEXT("texCoord")].IsInt())
        {
            data_ptr->texCoord = _JsonValue[GLTFTEXT("texCoord")].GetInt();
        }
        _pData = data_ptr;
        return true;
    }

    bool operator>>(const std::shared_ptr<STextureInfo>& _pData, GLTFCharValue& _JsonValue)
    {
        if (!_pData || !g_json_doc_ptr) return false;
        _JsonValue.SetObject();
        {
            const std::shared_ptr<SGlTFProperty> super_ptr = _pData;
            if (!(super_ptr >> _JsonValue)) return false;
        }
        if (!!_pData->index)
        {
            GLTFCharValue json_value;
            if (!(_pData->index >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("index"), json_value, g_json_doc_ptr->GetAllocator());
        }
        {
            GLTFCharValue json_value;
            if (!(_pData->texCoord >> json_value)) return false;
            _JsonValue.AddMember(GLTFTEXT("texCoord"), json_value, g_json_doc_ptr->GetAllocator());
        }
        return true;
    }

    bool operator<<(std::vector<std::shared_ptr<STextureInfo>>& _vDatas, const GLTFCharValue& _JsonValue)
    {
        return operator<< <std::shared_ptr<STextureInfo>>(_vDatas, _JsonValue);
    }

    bool operator>>(const std::vector<std::shared_ptr<STextureInfo>>& _vDatas, GLTFCharValue& _JsonValue)
    {
        return operator>> <std::shared_ptr<STextureInfo>>(_vDatas, _JsonValue);
    }

}
