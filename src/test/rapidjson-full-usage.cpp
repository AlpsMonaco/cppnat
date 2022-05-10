#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <iostream>
#include <streambuf>

using namespace rapidjson;

const char *example = R"(
{
    "i": 1,
    "s": "string",
    "a": [
        1,
        2,
        3
    ],
    "o": {
        "k": "v",
        "k2": "v2"
    },
    "ao": [
        {
            "k": "v",
            "k2": "v2"
        },
        {
            "k": "v",
            "k2": "v2"
        }
    ]
}
)";

void Parse(Document &d)
{
    d.Parse(example);
    assert(!d.HasParseError());
    std::cout << d["i"].GetInt() << std::endl;
    std::cout << d["s"].GetString() << std::endl;
    assert(d["a"].IsArray());
    const Value &arr = d["a"];
    for (SizeType i = 0; i < arr.Size(); i++)
        std::cout << arr[i].GetInt() << std::endl;
    assert(d["o"].IsObject());
    const Value &obj = d["o"];
    std::cout << obj["k"].GetString() << std::endl;
    std::cout << obj["k2"].GetString() << std::endl;
    assert(d["ao"].IsArray());
    const Value &arr2 = d["ao"];
    for (SizeType i = 0; i < arr2.Size(); i++)
    {
        assert(arr2[i].IsObject());
        const Value &obj2 = arr2[i];
        std::cout << obj2["k"].GetString() << std::endl;
        std::cout << obj2["k2"].GetString() << std::endl;
    }
}

void Marshal(Document &d, Document &d2)
{

    d.SetObject();
    d.AddMember("i", 1, d.GetAllocator());
    d.AddMember("s", StringRef("string"), d.GetAllocator());
    Value arr(kArrayType);
    arr.PushBack(1, d.GetAllocator());
    arr.PushBack(2, d.GetAllocator());
    arr.PushBack(3, d.GetAllocator());
    d.AddMember("a", arr, d.GetAllocator());
    Value obj(kObjectType);
    obj.AddMember("k", StringRef("v"), d.GetAllocator());
    obj.AddMember("k2", StringRef("v2"), d.GetAllocator());
    obj.AddMember("k3", "v3", d.GetAllocator());
    std::string v4("v4");
    obj.AddMember("v4", StringRef(v4.c_str()), d.GetAllocator());
    d.AddMember("o", obj, d.GetAllocator());
    Value arr2(kArrayType);
    Value obj2(kObjectType);
    obj2.AddMember("k", StringRef("v"), d.GetAllocator());
    obj2.AddMember("k2", StringRef("v2"), d.GetAllocator());
    arr2.PushBack(obj2, d.GetAllocator());
    obj2.SetObject();
    obj2.AddMember("k", StringRef("v"), d.GetAllocator());
    obj2.AddMember("k2", StringRef("v2"), d.GetAllocator());
    obj2.AddMember("k3", d2["a"].GetArray(), d.GetAllocator());
    arr2.PushBack(obj2, d.GetAllocator());
    arr2.PushBack(StringRef(v4.c_str()), d.GetAllocator());
    d.AddMember("ao", arr2, d.GetAllocator());
    Value s;
    s.SetString("");
    d.AddMember("s11111", s, d.GetAllocator());
    // output json
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);
    std::cout << buffer.GetString() << std::endl;

    // reuse writer
    d.SetObject();
    buffer.Clear();
    writer.Reset(buffer);
    d2.Accept(writer);
    std::cout << buffer.GetString() << std::endl;
}

void Marshal2()
{
    rapidjson::StringBuffer strBuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);

    writer.StartObject();

    // 1. 整数类型
    writer.Key("Int");
    writer.Int(1);

    // 2. 浮点类型
    writer.Key("Double");
    writer.Double(12.0000001);

    // 3. 字符串类型
    writer.Key("String");
    writer.String("This is a string");

    // 4. 结构体类型
    writer.Key("Object");
    writer.StartObject();
    writer.Key("key");
    writer.String("val");
    writer.Key("key2");
    writer.Int(25);
    writer.EndObject();

    // 5. 数组类型
    // 5.1 整型数组
    writer.Key("IntArray");
    writer.StartArray();
    //顺序写入即可
    writer.Int(10);
    writer.Int(20);
    writer.Int(30);
    writer.EndArray();

    // 5.2 浮点型数组
    writer.Key("DoubleArray");
    writer.StartArray();
    for (int i = 1; i < 4; i++)
    {
        writer.Double(i * 1.0);
    }
    writer.EndArray();

    // 5.3 字符串数组
    writer.Key("StringArray");
    writer.StartArray();
    writer.String("one");
    writer.String("two");
    writer.String("three");
    writer.EndArray();

    // 5.4 混合型数组
    //这说明了，一个json数组内容是不限制类型的
    writer.Key("MixedArray");
    writer.StartArray();
    writer.String("one");
    writer.Int(50);
    writer.Bool(false);
    writer.Double(12.005);
    writer.EndArray();

    // 5.5 结构体数组
    writer.Key("People");
    writer.StartArray();
    for (int i = 0; i < 3; i++)
    {
        writer.StartObject();
        writer.Key("key");
        writer.String("val");
        writer.Key("key2");
        writer.Int(i * 10);
        writer.Key("sex");
        writer.Bool((i % 2) == 0);
        writer.EndObject();
    }
    writer.EndArray();
    writer.EndObject();

    std::cout << strBuf.GetString() << std::endl;
}

int main(int argc, char **argv)
{
    Document d;
    Document d2;
    Parse(d);
    Marshal(d2, d);
    Marshal2();
}