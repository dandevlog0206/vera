#pragma once

#include "../../../include/vera/asset/asset_loader.h"
#include "../../../include/vera/util/result_message.h"
#include "../../../include/vera/util/stack_allocator.h"
#include <string_view>
#include <fstream>
#include <vector>

#define FBX_HEADER_MAGIC "Kaydara FBX Binary  \x00\x1a\x00"
#define FBX_FOOTER_MAGIC "\xF8\x5A\x8C\x6A\xDE\xF5\xD9\x7E\xEC\xE9\x0C\xE3\x75\x8F\x29\x0B"
#define FBX_FOOTER_ID "\xFA\xBC\xAB\x09\xD0\xC8\xD4\x66\xB1\x76\xFB\x83\x1C\xF7\x26\x7E"

#define FBX_FORWARD_RESULT(expression)                                \
	do {                                                              \
		if (auto result = expression; result != AssetResult::Success) \
			return result;                                            \
	} while (0)

#define FBX_CHECK_ALLOC(ptr)                                                  \
	while (!ptr) {                                                            \
		return { AssetResult::AllocationFailed, "memory allocation failed" }; \
	} while (0)

VERA_NAMESPACE_BEGIN

static bool memvcmp(const void* ptr, size_t length, uint8_t value)
{
	const uint8_t* bytes = reinterpret_cast<const uint8_t*>(ptr);
	
	for (size_t i = 0; i < length; ++i)
		if (bytes[i] != value) return false;
	return true;
}

struct FBXLoader
{
	typedef ResultMessage<AssetResult> Result;

	enum PropertyTypeCode : char {
		BYTE         = 'Z',
		SHORT        = 'Y',
		BOOL         = 'B',
		CHAR         = 'C',
		INT          = 'I',
		FLOAT        = 'F',
		DOUBLE       = 'D',
		LONG         = 'L',
		BINARY       = 'R',
		STRING       = 'S',
		ARRAY_BOOL   = 'b',
		ARRAY_UBYTE  = 'c',
		ARRAY_INT    = 'i',
		ARRAY_LONG   = 'l',
		ARRAY_FLOAT  = 'f',
		ARRAY_DOUBLE = 'd',

		// compressed array types (non standard)
		ARRAY_COMP_BOOL   = 'z' + 1,
		ARRAY_COMP_UBYTE  = 'z' + 2,
		ARRAY_COMP_INT    = 'z' + 3,
		ARRAY_COMP_LONG   = 'z' + 4,
		ARRAY_COMP_FLOAT  = 'z' + 5,
		ARRAY_COMP_DOUBLE = 'z' + 6
	};

	struct PropertyRecord
	{
		PropertyTypeCode type;
	};

	typedef PropertyRecord* PropertyRecordPtr;

	template <PropertyTypeCode Code>
	struct PropertyRecordType
	{
		static_assert("invalid property type code");
	};

	template <>
	struct PropertyRecordType<BYTE> : PropertyRecord
	{
		int8_t value;
	};

	template <>
	struct PropertyRecordType<SHORT> : PropertyRecord
	{
		int16_t value;
	};

	template <>
	struct PropertyRecordType<BOOL> : PropertyRecord
	{
		bool value;
	};

	template <>
	struct PropertyRecordType<CHAR> : PropertyRecord
	{
		char value;
	};

	template <>
	struct PropertyRecordType<INT> : PropertyRecord
	{
		int32_t value;
	};

	template <>
	struct PropertyRecordType<FLOAT> : PropertyRecord
	{
		float value;
	};

	template <>
	struct PropertyRecordType<DOUBLE> : PropertyRecord
	{
		double value;
	};

	template <>
	struct PropertyRecordType<LONG> : PropertyRecord
	{
		int64_t value;
	};

	template <>
	struct PropertyRecordType<BINARY> : PropertyRecord
	{
		uint32_t length;
		uint8_t* value;
	};

	template <>
	struct PropertyRecordType<STRING> : PropertyRecord
	{
		uint32_t    length;
		const char* value;
	};

	template <>
	struct PropertyRecordType<ARRAY_BOOL> : PropertyRecord
	{
		uint32_t length;
		bool*    value;
	};

	template <>
	struct PropertyRecordType<ARRAY_UBYTE> : PropertyRecord
	{
		uint32_t length;
		uint8_t* value;
	};

	template <>
	struct PropertyRecordType<ARRAY_INT> : PropertyRecord
	{
		uint32_t length;
		int32_t* value;
	};

	template <>
	struct PropertyRecordType<ARRAY_LONG> : PropertyRecord
	{
		uint32_t length;
		int64_t* value;
	};

	template <>
	struct PropertyRecordType<ARRAY_FLOAT> : PropertyRecord
	{
		uint32_t length;
		float*   value;
	};

	template <>
	struct PropertyRecordType<ARRAY_DOUBLE> : PropertyRecord
	{
		uint32_t length;
		double*  value;
	};

	template <>
	struct PropertyRecordType<ARRAY_COMP_BOOL> : PropertyRecord
	{
		uint32_t length;
		uint32_t size;
		uint8_t* value;
	};

	template <>
	struct PropertyRecordType<ARRAY_COMP_UBYTE> : PropertyRecord
	{
		uint32_t length;
		uint32_t size;
		uint8_t* value;
	};
	template <>
	struct PropertyRecordType<ARRAY_COMP_INT> : PropertyRecord
	{
		uint32_t length;
		uint32_t size;
		uint8_t* value;
	};

	template <>
	struct PropertyRecordType<ARRAY_COMP_LONG> : PropertyRecord
	{
		uint32_t length;
		uint32_t size;
		uint8_t* value;
	};

	template <>
	struct PropertyRecordType<ARRAY_COMP_FLOAT> : PropertyRecord
	{
		uint32_t length;
		uint32_t size;
		uint8_t* value;
	};

	template <>
	struct PropertyRecordType<ARRAY_COMP_DOUBLE> : PropertyRecord
	{
		uint32_t length;
		uint32_t size;
		uint8_t* value;
	};

	class NodeRecord
	{
	public:
		const char*      name;
		uint32_t         propertyCount;
		PropertyRecord** properties;
		uint32_t         childCount;
		NodeRecord*      childs;
	};

#pragma pack(push, 1)
	struct Header
	{
		char     magic[23];
		uint32_t version;
	};

	struct PropertyRecordArrayRaw
	{
		uint32_t arrayLength;
		uint32_t encoding;
		uint32_t compressedLength;
	};

	struct NodeRecord32Raw
	{
		uint32_t endOffset;
		uint32_t propertyCount;
		uint32_t propertyListLength;
		uint8_t  nameLength;
	};

	struct NodeRecord64Raw
	{
		uint64_t endOffset;
		uint64_t propertyCount;
		uint64_t propertyListLength;
		uint8_t  nameLength;
	};
#pragma pack(pop, 1)

	static Result load(AssetLoader& loader, std::string_view path)
	{
		NodeRecord     root_node = {};
		StackAllocator allocator;
		Header         header;

		std::ifstream input_file(path.data(), std::ios::binary);

		if (!input_file.is_open())
			return AssetResult::FileNotFound;

		input_file.read(reinterpret_cast<char*>(&header), sizeof(Header));
		if (memcmp(header.magic, FBX_HEADER_MAGIC, sizeof(header.magic)) != 0)
			return { AssetResult::InvalidMagic, "invalid header magic" };
		
		if (header.version < 7500)
			FBX_FORWARD_RESULT(parse_node<NodeRecord32Raw>(input_file, allocator, root_node));
		else
			FBX_FORWARD_RESULT(parse_node<NodeRecord64Raw>(input_file, allocator, root_node));

		FBX_FORWARD_RESULT(check_footer(input_file, header.version));

		input_file.close();

		// root_node.name = "Root Node";
		// print_node(root_node);

		
		
		loader.m_file_format = AssetFileFormat::FBX;
		loader.m_version     = Version(
			(header.version / 1000) % 100,
			(header.version / 100) % 10,
			header.version % 100);

		return AssetResult::Success;
	}

private:
	static const char* get_property_code_name(PropertyTypeCode code)
	{
		switch (code) {
		case BYTE:              return "BYTE";
		case SHORT:             return "SHORT";
		case BOOL:              return "BOOL";
		case CHAR:              return "CHAR";
		case INT:               return "INT";
		case FLOAT:             return "FLOAT";
		case DOUBLE:            return "DOUBLE";
		case LONG:              return "LONG";
		case BINARY:            return "BINARY";
		case STRING:            return "STRING";
		case ARRAY_BOOL:        return "ARRAY_BOOL";
		case ARRAY_UBYTE:       return "ARRAY_UBYTE";
		case ARRAY_INT:         return "ARRAY_INT";
		case ARRAY_LONG:        return "ARRAY_LONG";
		case ARRAY_FLOAT:       return "ARRAY_FLOAT";
		case ARRAY_DOUBLE:      return "ARRAY_DOUBLE";
		case ARRAY_COMP_BOOL:   return "ARRAY_COMP_BOOL";
		case ARRAY_COMP_UBYTE:  return "ARRAY_COMP_UBYTE";
		case ARRAY_COMP_INT:    return "ARRAY_COMP_INT";
		case ARRAY_COMP_LONG:   return "ARRAY_COMP_LONG";
		case ARRAY_COMP_FLOAT:  return "ARRAY_COMP_FLOAT";
		case ARRAY_COMP_DOUBLE: return "ARRAY_COMP_DOUBLE";
		};

		return "UNKNOWN";
	}

	static void print_node(const NodeRecord& node, int indent = 0)
	{
		for (int i = 0; i < indent; ++i)
			printf("  ");

		printf("Node: %s, props: %u, childs: %u\n", node.name, node.propertyCount, node.childCount);
		for (uint32_t i = 0; i < node.propertyCount; ++i) {
			for (int j = 0; j < indent + 1; ++j) printf("  ");
			printf("Prop: %s\n", get_property_code_name(node.properties[i]->type));
		}
		for (uint32_t i = 0; i < node.childCount; ++i)
			print_node(node.childs[i], indent + 1);
	}

	static Result check_footer(std::ifstream& input_file, uint32_t header_version)
	{
		char     footer_id[16];
		char     padding[16];
		char     zeros[4];
		char     static_padding[120];
		char     magic[16];
		size_t   padding_size;
		uint32_t version;

		input_file.read(reinterpret_cast<char*>(&footer_id), sizeof(footer_id));
		//if (memcmp(footer_id, FBX_FOOTER_ID, sizeof(footer_id)) != 0)
		//	return { AssetResult::InvalidID, "invalid footer id" };

		padding_size = 0x10 - input_file.tellg() % 0x10;

		input_file.read(reinterpret_cast<char*>(&padding), padding_size);
		if (!memvcmp(padding, padding_size, 0))
			return { AssetResult::InvalidPadding, "found non-zero values in alignment padding" };

		input_file.read(reinterpret_cast<char*>(&zeros), sizeof(zeros));
		if (!memvcmp(zeros, sizeof(zeros), 0))
			return { AssetResult::InvalidPadding, "found non-zero values after footer id" };

		input_file.read(reinterpret_cast<char*>(&version), sizeof(version));
		if (version != header_version)
			return { AssetResult::InvalidFormat, "footer version mismatch" };

		input_file.read(reinterpret_cast<char*>(&static_padding), sizeof(static_padding));
		if (!memvcmp(static_padding, sizeof(static_padding), 0))
			return { AssetResult::InvalidPadding, "found non-zero values in static padding" };
		
		input_file.read(reinterpret_cast<char*>(&magic), sizeof(magic));
		if (memcmp(magic, FBX_FOOTER_MAGIC, sizeof(magic)) != 0)
			return { AssetResult::InvalidMagic, "invalid footer magic" };

		return AssetResult::Success;
	}

	template <class NodeRecordType>
	static Result parse_node(std::ifstream& input_file, StackAllocator& allocator, NodeRecord& parent_node)
	{
		NodeRecordType raw_node;
		size_t         prop_end_offset;
		char*          name_buffer;

		if ((parent_node.childCount = count_child_node(input_file)) != 0) {
			parent_node.childs = allocator.allocate<NodeRecord>(parent_node.childCount);
			FBX_CHECK_ALLOC(parent_node.childs);
		}

		for (uint32_t i = 0; i < parent_node.childCount + 1; ++i) {
			input_file.read(reinterpret_cast<char*>(&raw_node), sizeof(NodeRecordType));

			if (raw_node.endOffset == 0) break;
			
			NodeRecord& node = parent_node.childs[i] = {};
			
			name_buffer = allocator.allocate<char>(raw_node.nameLength + 1);
			FBX_CHECK_ALLOC(name_buffer);

			// parse name
			input_file.read(name_buffer, raw_node.nameLength);
			name_buffer[raw_node.nameLength] = '\0';

			node.name = name_buffer;

			// parse properties
			prop_end_offset = raw_node.propertyListLength + input_file.tellg();

			if ((node.propertyCount = raw_node.propertyCount) != 0) {
				node.properties = allocator.allocate<PropertyRecordPtr>(raw_node.propertyCount);
				FBX_CHECK_ALLOC(node.properties);

				for (uint32_t i = 0; i < raw_node.propertyCount; ++i)
					FBX_FORWARD_RESULT(parse_property(input_file, allocator, &node.properties[i]));
			}

			input_file.seekg(prop_end_offset);

			if (input_file.tellg() != prop_end_offset)
				return { AssetResult::InvalidFormat, "property record end offset mismatch" };

			if (input_file.tellg() == raw_node.endOffset) continue;

			// parse child nodes
			FBX_FORWARD_RESULT(parse_node<NodeRecordType>(input_file, allocator, node));

			if (input_file.tellg() != raw_node.endOffset)
				return { AssetResult::InvalidFormat, "node record end offset mismatch" };
		}

		return AssetResult::Success;
	}

	static Result parse_property(std::ifstream& input_file, StackAllocator& allocator, PropertyRecordPtr* prop_ptr)
	{
		PropertyRecordArrayRaw raw_prop;
		uint32_t               length;
		char*                  string_buffer;

		switch (static_cast<PropertyTypeCode>(input_file.get())) {
		case BYTE: {
			auto* new_prop = allocator.allocate<PropertyRecordType<BYTE>>();
			FBX_CHECK_ALLOC(new_prop);
			new_prop->type = BYTE;
			input_file.read(reinterpret_cast<char*>(&new_prop->value), sizeof(int8_t));
			*prop_ptr = new_prop;
		} break;
		case SHORT: {
			auto* new_prop = allocator.allocate<PropertyRecordType<SHORT>>();
			FBX_CHECK_ALLOC(new_prop);
			new_prop->type = SHORT;
			input_file.read(reinterpret_cast<char*>(&new_prop->value), sizeof(int16_t));
			*prop_ptr = new_prop;
		} break;
		case BOOL: {
			auto* new_prop = allocator.allocate<PropertyRecordType<BOOL>>();
			FBX_CHECK_ALLOC(new_prop);
			new_prop->type = BOOL;
			input_file.read(reinterpret_cast<char*>(&new_prop->value), sizeof(bool));
			*prop_ptr = new_prop;
		} break;
		case CHAR: {
			auto* new_prop = allocator.allocate<PropertyRecordType<CHAR>>();
			FBX_CHECK_ALLOC(new_prop);
			new_prop->type = CHAR;
			input_file.read(reinterpret_cast<char*>(&new_prop->value), sizeof(char));
			*prop_ptr = new_prop;
		} break;
		case INT: {
			auto* new_prop = allocator.allocate<PropertyRecordType<INT>>();
			FBX_CHECK_ALLOC(new_prop);
			new_prop->type = INT;
			input_file.read(reinterpret_cast<char*>(&new_prop->value), sizeof(int32_t));
			*prop_ptr = new_prop;
		} break;
		case FLOAT: {
			auto* new_prop = allocator.allocate<PropertyRecordType<FLOAT>>();
			FBX_CHECK_ALLOC(new_prop);
			new_prop->type = FLOAT;
			input_file.read(reinterpret_cast<char*>(&new_prop->value), sizeof(float));
			*prop_ptr = new_prop;
		} break;
		case DOUBLE: {
			auto* new_prop = allocator.allocate<PropertyRecordType<DOUBLE>>();
			FBX_CHECK_ALLOC(new_prop);
			new_prop->type = DOUBLE;
			input_file.read(reinterpret_cast<char*>(&new_prop->value), sizeof(double));
			*prop_ptr = new_prop;
		} break;
		case LONG: {
			auto* new_prop = allocator.allocate<PropertyRecordType<LONG>>();
			FBX_CHECK_ALLOC(new_prop);
			new_prop->type = LONG;
			input_file.read(reinterpret_cast<char*>(&new_prop->value), sizeof(int64_t));
			*prop_ptr = new_prop;
		} break;
		case BINARY: {
			auto* new_prop = allocator.allocate<PropertyRecordType<BINARY>>();
			FBX_CHECK_ALLOC(new_prop);
			new_prop->type = BINARY;
			input_file.read(reinterpret_cast<char*>(&length), sizeof(uint32_t));
			new_prop->value = allocator.allocate<uint8_t>(length);
			FBX_CHECK_ALLOC(new_prop->value);
			input_file.read(reinterpret_cast<char*>(new_prop->value), length);
			*prop_ptr = new_prop;
		} break;
		case STRING: {
			auto* new_prop = allocator.allocate<PropertyRecordType<STRING>>();
			FBX_CHECK_ALLOC(new_prop);
			new_prop->type = STRING;
			input_file.read(reinterpret_cast<char*>(&length), sizeof(uint32_t));
			string_buffer = allocator.allocate<char>(length + 1);
			FBX_CHECK_ALLOC(string_buffer);
			input_file.read(string_buffer, length);
			string_buffer[length] = '\0';
			new_prop->length = length;
			new_prop->value  = string_buffer;
			*prop_ptr = new_prop;
		} break;
		case ARRAY_BOOL: {
			input_file.read(reinterpret_cast<char*>(&raw_prop), sizeof(PropertyRecordArrayRaw));
			if (raw_prop.encoding == 0) {
				auto* new_prop = allocator.allocate<PropertyRecordType<ARRAY_BOOL>>();
				FBX_CHECK_ALLOC(new_prop);
				new_prop->type   = ARRAY_BOOL;
				new_prop->length = raw_prop.arrayLength;
				new_prop->value  = allocator.allocate<bool>(raw_prop.arrayLength);
				FBX_CHECK_ALLOC(new_prop->value);
				input_file.read(reinterpret_cast<char*>(new_prop->value), raw_prop.arrayLength);
				*prop_ptr = new_prop;
			} else if (raw_prop.encoding == 1) {
				auto* new_prop = allocator.allocate<PropertyRecordType<ARRAY_COMP_BOOL>>();
				FBX_CHECK_ALLOC(new_prop);
				new_prop->type   = ARRAY_COMP_BOOL;
				new_prop->length = raw_prop.arrayLength;
				new_prop->size   = raw_prop.compressedLength;
				new_prop->value  = allocator.allocate<uint8_t>(raw_prop.compressedLength);
				FBX_CHECK_ALLOC(new_prop->value);
				input_file.read(reinterpret_cast<char*>(new_prop->value), raw_prop.compressedLength);
				*prop_ptr = new_prop;
			} else {
				return { AssetResult::InvalidFormat, "invalid array encoding"};
			}
		} break;
		case ARRAY_UBYTE: {
			input_file.read(reinterpret_cast<char*>(&raw_prop), sizeof(PropertyRecordArrayRaw));
			if (raw_prop.encoding == 0) {
				auto* new_prop = allocator.allocate<PropertyRecordType<ARRAY_UBYTE>>();
				FBX_CHECK_ALLOC(new_prop);
				new_prop->type   = ARRAY_UBYTE;
				new_prop->length = raw_prop.arrayLength;
				new_prop->value  = allocator.allocate<uint8_t>(raw_prop.arrayLength);
				FBX_CHECK_ALLOC(new_prop->value);
				input_file.read(reinterpret_cast<char*>(new_prop->value), raw_prop.arrayLength);
				*prop_ptr = new_prop;
			} else if (raw_prop.encoding == 1) {
				auto* new_prop = allocator.allocate<PropertyRecordType<ARRAY_COMP_UBYTE>>();
				FBX_CHECK_ALLOC(new_prop);
				new_prop->type   = ARRAY_COMP_UBYTE;
				new_prop->length = raw_prop.arrayLength;
				new_prop->size   = raw_prop.compressedLength;
				new_prop->value  = allocator.allocate<uint8_t>(raw_prop.compressedLength);
				FBX_CHECK_ALLOC(new_prop->value);
				input_file.read(reinterpret_cast<char*>(new_prop->value), raw_prop.compressedLength);
				*prop_ptr = new_prop;
			} else {
				return { AssetResult::InvalidFormat, "invalid array encoding"};
			}
		} break;
		case ARRAY_INT: {
			input_file.read(reinterpret_cast<char*>(&raw_prop), sizeof(PropertyRecordArrayRaw));
			if (raw_prop.encoding == 0) {
				auto* new_prop = allocator.allocate<PropertyRecordType<ARRAY_INT>>();
				FBX_CHECK_ALLOC(new_prop);
				new_prop->type   = ARRAY_INT;
				new_prop->length = raw_prop.arrayLength;
				new_prop->value  = allocator.allocate<int32_t>(raw_prop.arrayLength);
				FBX_CHECK_ALLOC(new_prop->value);
				input_file.read(reinterpret_cast<char*>(new_prop->value), raw_prop.arrayLength * sizeof(int32_t));
				*prop_ptr = new_prop;
			} else if (raw_prop.encoding == 1) {
				auto* new_prop = allocator.allocate<PropertyRecordType<ARRAY_COMP_INT>>();
				FBX_CHECK_ALLOC(new_prop);
				new_prop->type   = ARRAY_COMP_INT;
				new_prop->length = raw_prop.arrayLength;
				new_prop->size   = raw_prop.compressedLength;
				new_prop->value  = allocator.allocate<uint8_t>(raw_prop.compressedLength);
				FBX_CHECK_ALLOC(new_prop->value);
				input_file.read(reinterpret_cast<char*>(new_prop->value), raw_prop.compressedLength);
				*prop_ptr = new_prop;
			} else {
				return { AssetResult::InvalidFormat, "invalid array encoding"};
			}
		} break;
		case ARRAY_LONG: {
			input_file.read(reinterpret_cast<char*>(&raw_prop), sizeof(PropertyRecordArrayRaw));
			if (raw_prop.encoding == 0) {
				auto* new_prop = allocator.allocate<PropertyRecordType<ARRAY_LONG>>();
				FBX_CHECK_ALLOC(new_prop);
				new_prop->type   = ARRAY_LONG;
				new_prop->length = raw_prop.arrayLength;
				new_prop->value  = allocator.allocate<int64_t>(raw_prop.arrayLength);
				FBX_CHECK_ALLOC(new_prop->value);
				input_file.read(reinterpret_cast<char*>(new_prop->value), raw_prop.arrayLength * sizeof(int64_t));
				*prop_ptr = new_prop;
			} else if (raw_prop.encoding == 1) {
				auto* new_prop = allocator.allocate<PropertyRecordType<ARRAY_COMP_LONG>>();
				FBX_CHECK_ALLOC(new_prop);
				new_prop->type   = ARRAY_COMP_LONG;
				new_prop->length = raw_prop.arrayLength;
				new_prop->size   = raw_prop.compressedLength;
				new_prop->value  = allocator.allocate<uint8_t>(raw_prop.compressedLength);
				FBX_CHECK_ALLOC(new_prop->value);
				input_file.read(reinterpret_cast<char*>(new_prop->value), raw_prop.compressedLength);
				*prop_ptr = new_prop;
			} else {
				return { AssetResult::InvalidFormat, "invalid array encoding"};
			}
		} break;
		case ARRAY_FLOAT: {
			input_file.read(reinterpret_cast<char*>(&raw_prop), sizeof(PropertyRecordArrayRaw));
			if (raw_prop.encoding == 0) {
				auto* new_prop = allocator.allocate<PropertyRecordType<ARRAY_FLOAT>>();
				FBX_CHECK_ALLOC(new_prop);
				new_prop->type   = ARRAY_FLOAT;
				new_prop->length = raw_prop.arrayLength;
				new_prop->value  = allocator.allocate<float>(raw_prop.arrayLength);
				FBX_CHECK_ALLOC(new_prop->value);
				input_file.read(reinterpret_cast<char*>(new_prop->value), raw_prop.arrayLength * sizeof(float));
				*prop_ptr = new_prop;
			} else if (raw_prop.encoding == 1) {
				auto* new_prop = allocator.allocate<PropertyRecordType<ARRAY_COMP_FLOAT>>();
				FBX_CHECK_ALLOC(new_prop);
				new_prop->type   = ARRAY_COMP_FLOAT;
				new_prop->length = raw_prop.arrayLength;
				new_prop->size   = raw_prop.compressedLength;
				new_prop->value  = allocator.allocate<uint8_t>(raw_prop.compressedLength);
				FBX_CHECK_ALLOC(new_prop->value);
				input_file.read(reinterpret_cast<char*>(new_prop->value), raw_prop.compressedLength);
				*prop_ptr = new_prop;
			} else {
				return { AssetResult::InvalidFormat, "invalid array encoding"};
			}
		} break;
		case ARRAY_DOUBLE: {
			input_file.read(reinterpret_cast<char*>(&raw_prop), sizeof(PropertyRecordArrayRaw));
			if (raw_prop.encoding == 0) {
				auto* new_prop = allocator.allocate<PropertyRecordType<ARRAY_DOUBLE>>();
				FBX_CHECK_ALLOC(new_prop);
				new_prop->type   = ARRAY_DOUBLE;
				new_prop->length = raw_prop.arrayLength;
				new_prop->value  = allocator.allocate<double>(raw_prop.arrayLength);
				FBX_CHECK_ALLOC(new_prop->value);
				input_file.read(reinterpret_cast<char*>(new_prop->value), raw_prop.arrayLength * sizeof(double));
				*prop_ptr = new_prop;
			} else if (raw_prop.encoding == 1) {
				auto* new_prop = allocator.allocate<PropertyRecordType<ARRAY_COMP_DOUBLE>>();
				FBX_CHECK_ALLOC(new_prop);
				new_prop->type   = ARRAY_COMP_DOUBLE;
				new_prop->length = raw_prop.arrayLength;
				new_prop->size   = raw_prop.compressedLength;
				new_prop->value  = allocator.allocate<uint8_t>(raw_prop.compressedLength);
				FBX_CHECK_ALLOC(new_prop->value);
				input_file.read(reinterpret_cast<char*>(new_prop->value), raw_prop.compressedLength);
				*prop_ptr = new_prop;
			} else {
				return { AssetResult::InvalidFormat, "invalid array encoding"};
			}
		} break;
		default:
			return { AssetResult::InvalidFormat, "invalid property record type"};
		}

		return AssetResult::Success;
	}

	static uint32_t count_child_node(std::ifstream& input_file)
	{
		size_t   offset = input_file.tellg();
		uint32_t result = 0;
		uint32_t end_offset;

		while (true) {
			input_file.read(reinterpret_cast<char*>(&end_offset), sizeof(uint32_t));

			if (end_offset == 0) break;

			input_file.seekg(end_offset);

			result++;
		}

		input_file.seekg(offset);

		return result;
	}
};

VERA_NAMESPACE_END
