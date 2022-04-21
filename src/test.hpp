#include "packet.h"

#define CheckTestDataStart() \
	size_t array_size = 0;

#define CheckTestData(array)                 \
	array_size = array[0] + (array[1] << 8); \
	assert(array_size == sizeof(array));

void memcpy_log(char *dst, const char *src, size_t size)
{
	for (size_t i = 0; i < size; i++)
	{
		dst[i] = src[i];
		printf("%2d ", dst[i]);
	}
	printf("\n");
}

template <unsigned short total_size, unsigned short cmd = 1>
class BinaryDataGenerator
{
public:
	char data[total_size];
	static constexpr unsigned short size = total_size;
	BinaryDataGenerator()
	{
		assert(total_size >= 4);
		unsigned short &size = *reinterpret_cast<unsigned short *>(data);
		size = total_size;
		unsigned short &cmdRef = *reinterpret_cast<unsigned short *>(data + 2);
		cmdRef = cmd;
		for (size_t i = 4; i < total_size; i++)
			data[i] = '0';
	}
};

class BinaryDataMerger
{

public:
	size_t index;
	char *buffer;

	BinaryDataMerger() : index(0), buffer(nullptr) {}
	~BinaryDataMerger()
	{
		if (buffer)
			free(buffer);
	}

	template <typename DataSource>
	inline void Merge(const DataSource &dc)
	{
		if (buffer == nullptr)
			buffer = (char *)malloc(dc.size);
		else
			buffer = (char *)realloc(buffer, index + dc.size);
		for (size_t i = 0; i < dc.size; i++)
			buffer[index++] = dc.data[i];
	}

	template <typename DataSource, typename... Args>
	inline void Merge(const DataSource &dc, Args... args)
	{
		Merge(dc);
		Merge(args...);
	}

protected:
};

void TestPacket()
{
	using namespace cppnat;
	using Protocol = PacketProtocol<unsigned short, unsigned short, 128>;
	using PacketParser = PacketStreamer<Protocol>;
	using Packet = PacketParser::PacketType;
	using TestData_4 = BinaryDataGenerator<4, 4>;
	using TestData_5 = BinaryDataGenerator<5, 5>;
	using TestData_6 = BinaryDataGenerator<6, 6>;
	using TestData_7 = BinaryDataGenerator<7, 7>;
	using TestData_8 = BinaryDataGenerator<8, 8>;
	using TestData_9 = BinaryDataGenerator<9, 9>;
	using TestData_5 = BinaryDataGenerator<5, 5>;
	using TestData_6 = BinaryDataGenerator<6, 6>;
	using TestData_7 = BinaryDataGenerator<7, 7>;
	using TestData_8 = BinaryDataGenerator<8, 8>;
	using TestData_9 = BinaryDataGenerator<9, 9>;
	using TestData_10 = BinaryDataGenerator<10, 10>;
	using TestData_11 = BinaryDataGenerator<11, 11>;
	using TestData_12 = BinaryDataGenerator<12, 12>;
	using TestData_13 = BinaryDataGenerator<13, 13>;
	using TestData_14 = BinaryDataGenerator<14, 14>;
	using TestData_15 = BinaryDataGenerator<15, 15>;
	using TestData_16 = BinaryDataGenerator<16, 16>;
	using TestData_17 = BinaryDataGenerator<17, 17>;
	using TestData_18 = BinaryDataGenerator<18, 18>;
	using TestData_19 = BinaryDataGenerator<19, 19>;
	using TestData_20 = BinaryDataGenerator<20, 20>;
	using TestData_21 = BinaryDataGenerator<21, 21>;
	using TestData_22 = BinaryDataGenerator<22, 22>;
	using TestData_23 = BinaryDataGenerator<23, 23>;
	using TestData_24 = BinaryDataGenerator<24, 24>;
	using TestData_25 = BinaryDataGenerator<25, 25>;
	using TestData_26 = BinaryDataGenerator<26, 26>;
	using TestData_27 = BinaryDataGenerator<27, 27>;
	using TestData_28 = BinaryDataGenerator<28, 28>;
	using TestData_29 = BinaryDataGenerator<29, 29>;
	using TestData_30 = BinaryDataGenerator<30, 30>;
	using TestData_31 = BinaryDataGenerator<31, 31>;
	using TestData_32 = BinaryDataGenerator<32, 32>;
	using TestData_33 = BinaryDataGenerator<33, 33>;
	using TestData_34 = BinaryDataGenerator<34, 34>;
	using TestData_35 = BinaryDataGenerator<35, 35>;
	using TestData_36 = BinaryDataGenerator<36, 36>;
	using TestData_37 = BinaryDataGenerator<37, 37>;
	using TestData_38 = BinaryDataGenerator<38, 38>;
	using TestData_39 = BinaryDataGenerator<39, 39>;
	using TestData_40 = BinaryDataGenerator<40, 40>;
	using TestData_41 = BinaryDataGenerator<41, 41>;
	using TestData_42 = BinaryDataGenerator<42, 42>;
	using TestData_43 = BinaryDataGenerator<43, 43>;
	using TestData_44 = BinaryDataGenerator<44, 44>;
	using TestData_45 = BinaryDataGenerator<45, 45>;
	using TestData_46 = BinaryDataGenerator<46, 46>;
	using TestData_47 = BinaryDataGenerator<47, 47>;
	using TestData_48 = BinaryDataGenerator<48, 48>;
	using TestData_49 = BinaryDataGenerator<49, 49>;
	using TestData_50 = BinaryDataGenerator<50, 50>;
	using TestData_51 = BinaryDataGenerator<51, 51>;
	using TestData_52 = BinaryDataGenerator<52, 52>;
	using TestData_53 = BinaryDataGenerator<53, 53>;
	using TestData_54 = BinaryDataGenerator<54, 54>;
	using TestData_55 = BinaryDataGenerator<55, 55>;
	using TestData_56 = BinaryDataGenerator<56, 56>;

	BinaryDataMerger merger;
	merger.Merge(TestData_4(), TestData_5(), TestData_6(), TestData_7(), TestData_8(), TestData_9(), TestData_5(), TestData_6(), TestData_7(), TestData_8(), TestData_9(), TestData_10(), TestData_11(), TestData_12(), TestData_13(), TestData_14(), TestData_15(), TestData_16(), TestData_17(), TestData_18(), TestData_19(), TestData_20(), TestData_21(), TestData_22(), TestData_23(), TestData_24(), TestData_25(), TestData_26(), TestData_27(), TestData_28(), TestData_29(), TestData_30(), TestData_31(), TestData_32(), TestData_33(), TestData_34(), TestData_35(), TestData_36(), TestData_37(), TestData_38(), TestData_39(), TestData_40(), TestData_41(), TestData_42(), TestData_43(), TestData_44(), TestData_45(), TestData_46(), TestData_47(), TestData_48(), TestData_49(), TestData_50(), TestData_51(), TestData_52(), TestData_53(), TestData_54(), TestData_55(), TestData_56());

	class TestReader : public Reader
	{
	public:
		int index;
		TestReader(BinaryDataMerger &merger) : merger(merger), index(0) {}
		Reader::Size Read(char *buffer, size_t size) override
		{
			static constexpr size_t sendSize = 7;
			if (index >= merger.index)
				return 0;
			size_t cSize = merger.index - index < sendSize ? merger.index - index : sendSize;
			memcpy_log(buffer, merger.buffer + (index), cSize);
			index += sendSize;
			return sendSize;
		}

	protected:
		BinaryDataMerger &merger;
	};

	TestReader reader(merger);
	PacketParser parser(reader);
	while (parser.Next())
	{
		PacketParser::PacketType &packet = parser.GetPacket();
		// Println(packet.size);
		// Println(packet.cmd);
		Print(packet.size, packet.cmd, " ");
		for (int i = 4; i < packet.size - 4; ++i)
			Print(int(packet.data[i]), " ");
		Println("");
	}
}