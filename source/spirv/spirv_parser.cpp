#include "spirv_parser.h"

#include "../../include/vera/core/logger.h"
#include "../../include/vera/util/stopwatch.h"

VERA_NAMESPACE_BEGIN

static size_t count_consecutive_op(spv_inst first, spv_inst last, spv::Op op)
{
	size_t count = 0;

	while (first != last && first.op() == op) {
		++count;
		++first;
	}

	return count;
}

static spv_inst skip_debug_instructions(spv_inst inst, spv_inst last)
{
	while (inst != last) {
		switch (inst.op()) {
		case spv::Op::OpSourceContinued:
		case spv::Op::OpSource:
		case spv::Op::OpSourceExtension:
		case spv::Op::OpName:
		case spv::Op::OpMemberName:
		case spv::Op::OpString:
		case spv::Op::OpLine:
		case spv::Op::OpNoLine:
		case spv::Op::OpModuleProcessed:
			++inst;
			break;
		default:
			return inst;
		}
	}
	return inst;
}

static spv_inst skip_annotation_instructions(spv_inst inst, spv_inst last)
{
	while (inst != last) {
		switch (inst.op()) {
		case spv::Op::OpDecorate:
		case spv::Op::OpMemberDecorate:
		case spv::Op::OpDecorationGroup:
		case spv::Op::OpGroupDecorate:
		case spv::Op::OpGroupMemberDecorate:
		case spv::Op::OpDecorateId:
		case spv::Op::OpDecorateString:
		case spv::Op::OpMemberDecorateString:
			++inst;
			break;
		default:
			return inst;
		}
	}
	return inst;
}

SpvParser::SpvParser() :
	spirvCode() {}

SpvParser::SpvParser(std::vector<uint32_t>&& spirv_code) :
	spirvStorage(std::move(spirv_code)),
	spirvCode(spirvStorage)
{
	parse();
}

SpvParser::SpvParser(array_view<uint32_t> spirv_code) :
	spirvCode(spirv_code)
{
	parse();
}

SpvParser::~SpvParser()
{
}

void SpvParser::parse()
{
	if (spirvCode[0] != spv::MagicNumber)
		throw Exception("invalid SPIR-V magic number");
	if (spirvCode.size() < 5)
		throw Exception("invalid SPIR-V code size");

	uint32_t spirv_version = spirvCode[1];
	uint32_t generator     = spirvCode[2];
	uint32_t id_bound      = spirvCode[3];
	
	version = Version(
		(spirv_version >> 16) & 0xff,
		(spirv_version >> 8) & 0xff,
		0);

	generatorMagic = spirvCode[2];

	StopWatch sw;

	nodes.resize(id_bound);

	sw.start();


	spv_inst first(spirvCode.data() + 5);
	spv_inst last(spirvCode.data() + spirvCode.size());

	parseNodes(first, last);

	sw.stop();

	Logger::info("parsing took {:.3f} ms", sw.get_ms());
}

void SpvParser::parseNodes(spv_inst first, spv_inst last)
{
	spv_inst inst = first;

	size_t cap_count = count_consecutive_op(inst, last, spv::Op::OpCapability);
	capabilities.reserve(cap_count);
	while (inst != last && inst.op() == spv::Op::OpCapability) {
		auto capability = inst.get_enum<spv::Capability>(1);
		capabilities.push_back(capability);
		++inst;
	}
	
	size_t ext_count = count_consecutive_op(inst, last, spv::Op::OpExtension);
	extensionNames.reserve(ext_count);
	while (inst != last && inst.op() == spv::Op::OpExtension) {
		std::string_view ext_name = inst.get_string(1);
		extensionNames.push_back(alloc_string(ext_name));
		++inst;
	}

	while (inst != last) {
		switch (inst.op()) {
		//////// Mode setting section begin ////////
		case spv::OpExtInstImport: {
			auto* new_node = alloc<SpvExtInstNode>();
			new_node->op          = spv::Op::OpExtInstImport;
			new_node->id          = inst.get_id(1);
			new_node->extInstName = alloc_string(inst.get_string(2));
			setNode(new_node);
		} break;
		case spv::OpMemoryModel: {
			memoryModel = inst.get_enum<spv::MemoryModel>(1);
		} break;
		case spv::OpEntryPoint: {
			uint32_t end_off;
			auto* new_node = alloc<SpvEntryPointNode>();
			new_node->op             = spv::Op::OpEntryPoint;
			new_node->executionModel = inst.get_enum<spv::ExecutionModel>(1);
			new_node->id             = inst.get_id(2);
			new_node->entryPointName = alloc_string(inst.get_string(3, end_off));
			new_node->interfaceIds   = inst.get_array<spv::Id>(end_off);
			setNode(new_node);
		} break;
		case spv::OpExecutionMode: {

		} break;
		case spv::OpExecutionModeId: {

		} break;
		//////// Mode setting section end ////////
		}
		++inst;
	}

	spv_inst debug_first      = inst;
	spv_inst debug_last       = skip_debug_instructions(inst, last);
	spv_inst annotation_first = debug_last;
	spv_inst annotation_last  = skip_annotation_instructions(debug_last, last);

	for (inst = annotation_last; inst != last; ++inst) {
		switch (inst.op()) {
		case spv::OpTypeVoid: {
			auto* new_node = alloc<SpvBasicTypeNode>();
			new_node->op        = spv::Op::OpTypeVoid;
			new_node->id        = inst.get_id(1);
			new_node->basicType = SpvBasicType::Void;
			setTypeNode(new_node);
		} break;
		case spv::OpTypeBool: {
			auto* new_node = alloc<SpvBasicTypeNode>();
			new_node->op        = spv::Op::OpTypeBool;
			new_node->id        = inst.get_id(1);
			new_node->basicType = SpvBasicType::Bool;
			setTypeNode(new_node);
		} break;
		case spv::OpTypeInt: {
			auto* new_node = alloc<SpvBasicTypeNode>();
			new_node->op = spv::Op::OpTypeInt;
			new_node->id = inst.get_id(1);
			parseIntType(new_node, inst);
			setTypeNode(new_node);
		} break;
		case spv::OpTypeFloat: {
			auto* new_node = alloc<SpvBasicTypeNode>();
			new_node->op = spv::Op::OpTypeFloat;
			new_node->id = inst.get_id(1);
			parseFloatType(new_node, inst);
			setTypeNode(new_node);
		} break;
		case spv::OpTypeVector: {
			auto* new_node = alloc<SpvBasicTypeNode>();
			new_node->op = spv::Op::OpTypeVector;
			new_node->id = inst.get_id(1);
			parseVectorType(new_node, inst);
			setTypeNode(new_node);
		} break;
		case spv::OpTypeMatrix: {
			auto* new_node = alloc<SpvBasicTypeNode>();
			new_node->op = spv::Op::OpTypeMatrix;
			new_node->id = inst.get_id(1);
			parseMatrixType(new_node, inst);
			setTypeNode(new_node);
		} break;
		case spv::OpTypeImage: {
			auto* new_node = alloc<SpvImageTypeNode>();
			new_node->op = spv::Op::OpTypeImage;
			new_node->id = inst.get_id(1);
			parseImageType(new_node, inst);
			setTypeNode(new_node);
		} break;
		case spv::OpTypeSampler:
		case spv::OpTypeSampledImage: {
			auto* new_node = alloc<SpvTypeNode>();
			new_node->op = spv::Op::OpTypeSampledImage;
			new_node->id = inst.get_id(1);
			setTypeNode(new_node);
		} break;
		}
	}

	parseDebugSection(debug_first, debug_last);
	parseAnnotationSection(annotation_first, annotation_last);
}

void SpvParser::parseDebugSection(spv_inst first, spv_inst last)
{
	for (spv_inst inst = first; inst != last; ++inst) {
		switch (inst.op()) {
		case spv::OpSourceContinued: {
		} break;
		case spv::OpSource: {

		} break;
		case spv::OpSourceExtension: {

		} break;
		case spv::OpName: {
			spv::Id          id   = inst.get_id(1);
			std::string_view name = alloc_string(inst.get_string(2));
			auto*            node = findNode<SpvNode>(id);

			if (node == nullptr)
				throw Exception("node not found for name assignment");
			if (node->meta == nullptr)
				node->meta = alloc<SpvNodeMeta>();

			node->meta->name = name;
		} break;
		case spv::OpString: {
			auto* new_node   = alloc<SpvStringNode>();
			new_node->op     = spv::Op::OpString;
			new_node->id     = inst.get_id(1);
			new_node->string = alloc_string(inst.get_string(2));
			setNode(new_node);
		} break;
		case spv::OpMemberName: {
			spv::Id              id         = inst.get_id(1);
			uint32_t             member_idx = inst.get_u32(2);
			std::string_view     name       = alloc_string(inst.get_string(3));
			SpvStructTypeMember& member = findNode<SpvStructTypeNode>(id)->members[member_idx];
			member.name = name;
		} break;
		}
	}
}

void SpvParser::parseAnnotationSection(spv_inst first, spv_inst last)
{
	for (spv_inst inst = first; inst != last; ++inst) {
		switch (inst.op()) {

		}
	}
}

void SpvParser::setNode(SpvNode* new_node)
{
	VERA_ASSERT_MSG(new_node->id < nodes.size(), "SPIR-V ID is out of range");
	VERA_ASSERT_MSG(nodes[new_node->id] == nullptr, "SPIR-V node with the same ID already exists");
	
	nodes[new_node->id] = new_node;
}

void SpvParser::setTypeNode(SpvTypeNode* new_node)
{
	setNode(new_node);
	typeNodes.push_back(new_node);
}

void SpvParser::setDecoration(spv_inst inst)
{
	spv::Id         id         = inst.get_id(1);
	uint32_t        off        = inst.op() == spv::OpMemberDecorate || inst.op() == spv::OpMemberDecorateString;
	spv::Decoration decoration = inst.get_enum<spv::Decoration>(off + 2);

	spv_decoration* deco = nullptr;

	if (off == 0) {
		auto* node = findNode<SpvNode>(id);

		if (node == nullptr)
			throw Exception("node not found for decoration");
		if (node->meta == nullptr)
			node->meta = alloc<SpvNodeMeta>();

		deco = &node->meta->decoration;
	} else {
		auto*    struct_node = findNode<SpvStructTypeNode>(id);
		uint32_t member_idx  = inst.get_u32(2);

		if (struct_node == nullptr)
			throw Exception("struct type node not found for member decoration");
		if (member_idx >= struct_node->members.size())
			throw Exception("member index out of range for member decoration");

		deco = &struct_node->members[member_idx].decoration;
	}

	switch (decoration) {
	case spv::DecorationSpecId:
	case spv::DecorationArrayStride:
	case spv::DecorationMatrixStride:
	case spv::DecorationStream:
	case spv::DecorationLocation:
	case spv::DecorationComponent:
	case spv::DecorationIndex:
	case spv::DecorationBinding:
	case spv::DecorationDescriptorSet:
	case spv::DecorationOffset:
	case spv::DecorationXfbBuffer:
	case spv::DecorationXfbStride:
	case spv::DecorationInputAttachmentIndex:
	case spv::DecorationAlignment:
	case spv::DecorationMaxByteOffset:
	case spv::DecorationSecondaryViewportRelativeNV: {
		uint32_t value = inst.get_u32(off + 3);
		deco->set(decoration, &memory, value);
	} break;
	case spv::DecorationUniformId:
	case spv::DecorationAlignmentId:
	case spv::DecorationMaxByteOffsetId:
	case spv::DecorationNodeSharesPayloadLimitsWithAMDX:
	case spv::DecorationNodeMaxPayloadsAMDX:
	case spv::DecorationPayloadNodeNameAMDX:
	case spv::DecorationPayloadNodeBaseIndexAMDX:
	case spv::DecorationPayloadNodeArraySizeAMDX:
	case spv::DecorationCounterBuffer: {
		spv::Id id = inst.get_id(off + 3);
		deco->set(decoration, &memory, id);
	} break;
	case spv::DecorationBuiltIn: {
		spv::BuiltIn builtin = inst.get_enum<spv::BuiltIn>(off + 3);
		deco->set(decoration, &memory, builtin);
	} break;
	case spv::DecorationFuncParamAttr: {
		auto func_param_attr = inst.get_enum<spv::FunctionParameterAttribute>(off + 3);
		deco->set(decoration, &memory, func_param_attr);
	} break;
	case spv::DecorationFPRoundingMode: {
		auto fp_rounding_mode = inst.get_enum<spv::FPRoundingMode>(off + 3);
		deco->set(decoration, &memory, fp_rounding_mode);
	} break;
	case spv::DecorationFPFastMathMode: {
		auto fp_fast_math_mode = inst.get_enum<spv::FPFastMathModeMask>(off + 3);
		deco->set(decoration, &memory, fp_fast_math_mode);
	} break;
	case spv::DecorationLinkageAttributes: {
		uint32_t         end_off;
		std::string_view name      = inst.get_string(off + 3, end_off);
		spv::LinkageType link_type = inst.get_enum<spv::LinkageType>(end_off);
		deco->set(decoration, &memory, name, link_type);
	} break;
	case spv::DecorationUserSemantic: {
		std::string_view name = inst.get_string(off + 3);
		deco->set(decoration, &memory, name);
	} break;
	default:
		deco->set(decoration);
	}
}

void SpvParser::parseIntType(SpvBasicTypeNode* node, spv_inst inst)
{
	uint32_t width = inst.get_u32(2);
	bool     sign  = inst.get_u32(3) != 0;

	switch (width) {
	case 8:  node->basicType = sign ? SpvBasicType::Int8 : SpvBasicType::UInt8;   return;
	case 16: node->basicType = sign ? SpvBasicType::Int16 : SpvBasicType::UInt16; return;
	case 32: node->basicType = sign ? SpvBasicType::Int32 : SpvBasicType::UInt32; return;
	case 64: node->basicType = sign ? SpvBasicType::Int64 : SpvBasicType::UInt64; return;
	}

	node->basicType = SpvBasicType::Unknown;
}

void SpvParser::parseFloatType(SpvBasicTypeNode* node, spv_inst inst)
{
	uint32_t width = inst.get_u32(2);
	
	switch (width) {
	case 8: {
		if (inst.length() != 4)
			throw Exception("invalid OpTypeFloat instruction for 8-bit float type");

		switch (inst.get_enum<spv::FPEncoding>(3)) {
			case 4214: node->basicType = SpvBasicType::Float8E4M3; return;
			case 4215: node->basicType = SpvBasicType::Float8E5M2; return;
		}
	} break;
	case 16: node->basicType = SpvBasicType::Float16; return;
	case 32: node->basicType = SpvBasicType::Float32; return;
	case 64: node->basicType = SpvBasicType::Float64; return;
	}

	node->basicType = SpvBasicType::Unknown;
}

void SpvParser::parseVectorType(SpvBasicTypeNode* node, spv_inst inst)
{
	spv::Id     comp_id    = inst.get_id(2);
	uint32_t    comp_count = inst.get_u32(3);
	const auto* comp_node  = findNode<SpvBasicTypeNode>(comp_id);
	
	if (comp_node == nullptr)
		throw Exception("component type node not found for vector type");

	switch (comp_count) {
	case 2:
		switch (comp_node->basicType) {
		case SpvBasicType::Bool:    node->basicType = SpvBasicType::Bool_2;    return;
		case SpvBasicType::Int8:    node->basicType = SpvBasicType::Int8_2;    return;
		case SpvBasicType::UInt8:   node->basicType = SpvBasicType::UInt8_2;   return;
		case SpvBasicType::Int16:   node->basicType = SpvBasicType::Int16_2;   return;
		case SpvBasicType::UInt16:  node->basicType = SpvBasicType::UInt16_2;  return;
		case SpvBasicType::Int32:   node->basicType = SpvBasicType::Int32_2;   return;
		case SpvBasicType::UInt32:  node->basicType = SpvBasicType::UInt32_2;  return;
		case SpvBasicType::Int64:   node->basicType = SpvBasicType::Int64_2;   return;
		case SpvBasicType::UInt64:  node->basicType = SpvBasicType::UInt64_2;  return;
		case SpvBasicType::Float16: node->basicType = SpvBasicType::Float16_2; return;
		case SpvBasicType::Float32: node->basicType = SpvBasicType::Float32_2; return;
		case SpvBasicType::Float64: node->basicType = SpvBasicType::Float64_2; return;
		} break;
	case 3:
		switch (comp_node->basicType) {
		case SpvBasicType::Bool:    node->basicType = SpvBasicType::Bool_3;    return;
		case SpvBasicType::Int8:    node->basicType = SpvBasicType::Int8_3;    return;
		case SpvBasicType::UInt8:   node->basicType = SpvBasicType::UInt8_3;   return;
		case SpvBasicType::Int16:   node->basicType = SpvBasicType::Int16_3;   return;
		case SpvBasicType::UInt16:  node->basicType = SpvBasicType::UInt16_3;  return;
		case SpvBasicType::Int32:   node->basicType = SpvBasicType::Int32_3;   return;
		case SpvBasicType::UInt32:  node->basicType = SpvBasicType::UInt32_3;  return;
		case SpvBasicType::Int64:   node->basicType = SpvBasicType::Int64_3;   return;
		case SpvBasicType::UInt64:  node->basicType = SpvBasicType::UInt64_3;  return;
		case SpvBasicType::Float16: node->basicType = SpvBasicType::Float16_3; return;
		case SpvBasicType::Float32: node->basicType = SpvBasicType::Float32_3; return;
		case SpvBasicType::Float64: node->basicType = SpvBasicType::Float64_3; return;
		} break;
	case 4:
		switch (comp_node->basicType) {
		case SpvBasicType::Bool:    node->basicType = SpvBasicType::Bool_4;    return;
		case SpvBasicType::Int8:    node->basicType = SpvBasicType::Int8_4;    return;
		case SpvBasicType::UInt8:   node->basicType = SpvBasicType::UInt8_4;   return;
		case SpvBasicType::Int16:   node->basicType = SpvBasicType::Int16_4;   return;
		case SpvBasicType::UInt16:  node->basicType = SpvBasicType::UInt16_4;  return;
		case SpvBasicType::Int32:   node->basicType = SpvBasicType::Int32_4;   return;
		case SpvBasicType::UInt32:  node->basicType = SpvBasicType::UInt32_4;  return;
		case SpvBasicType::Int64:   node->basicType = SpvBasicType::Int64_4;   return;
		case SpvBasicType::UInt64:  node->basicType = SpvBasicType::UInt64_4;  return;
		case SpvBasicType::Float16: node->basicType = SpvBasicType::Float16_4; return;
		case SpvBasicType::Float32: node->basicType = SpvBasicType::Float32_4; return;
		case SpvBasicType::Float64: node->basicType = SpvBasicType::Float64_4; return;
		} break;
	}
	
	throw Exception("unsupported vector type");
}

void SpvParser::parseMatrixType(SpvBasicTypeNode* node, spv_inst inst)
{
	spv::Id  comp_id     = inst.get_id(2);
	uint32_t comp_count  = inst.get_u32(3);
	const auto* comp_node = findNode<SpvBasicTypeNode>(comp_id);
	if (comp_node == nullptr)
		throw Exception("component type node not found for matrix type");

	switch (comp_count) {
	case 2:
		switch (comp_node->basicType) {
		case SpvBasicType::Bool_2:	  node->basicType = SpvBasicType::Bool_2x2;    return;
		case SpvBasicType::Bool_3:	  node->basicType = SpvBasicType::Bool_3x2;    return;
		case SpvBasicType::Bool_4:	  node->basicType = SpvBasicType::Bool_4x2;    return;
		case SpvBasicType::Int8_2:	  node->basicType = SpvBasicType::Int8_2x2;    return;
		case SpvBasicType::Int8_3:	  node->basicType = SpvBasicType::Int8_3x2;    return;
		case SpvBasicType::Int8_4:	  node->basicType = SpvBasicType::Int8_4x2;    return;
		case SpvBasicType::UInt8_2:	  node->basicType = SpvBasicType::UInt8_2x2;   return;
		case SpvBasicType::UInt8_3:	  node->basicType = SpvBasicType::UInt8_3x2;   return;
		case SpvBasicType::UInt8_4:	  node->basicType = SpvBasicType::UInt8_4x2;   return;
		case SpvBasicType::Int16_2:	  node->basicType = SpvBasicType::Int16_2x2;   return;
		case SpvBasicType::Int16_3:	  node->basicType = SpvBasicType::Int16_3x2;   return;
		case SpvBasicType::Int16_4:	  node->basicType = SpvBasicType::Int16_4x2;   return;
		case SpvBasicType::UInt16_2:  node->basicType = SpvBasicType::UInt16_2x2;  return;
		case SpvBasicType::UInt16_3:  node->basicType = SpvBasicType::UInt16_3x2;  return;
		case SpvBasicType::UInt16_4:  node->basicType = SpvBasicType::UInt16_4x2;  return;
		case SpvBasicType::Int32_2:	  node->basicType = SpvBasicType::Int32_2x2;   return;
		case SpvBasicType::Int32_3:	  node->basicType = SpvBasicType::Int32_3x2;   return;
		case SpvBasicType::Int32_4:	  node->basicType = SpvBasicType::Int32_4x2;   return;
		case SpvBasicType::UInt32_2:  node->basicType = SpvBasicType::UInt32_2x2;  return;
		case SpvBasicType::UInt32_3:  node->basicType = SpvBasicType::UInt32_3x2;  return;
		case SpvBasicType::UInt32_4:  node->basicType = SpvBasicType::UInt32_4x2;  return;
		case SpvBasicType::Int64_2:	  node->basicType = SpvBasicType::Int64_2x2;   return;
		case SpvBasicType::Int64_3:	  node->basicType = SpvBasicType::Int64_3x2;   return;
		case SpvBasicType::Int64_4:	  node->basicType = SpvBasicType::Int64_4x2;   return;
		case SpvBasicType::UInt64_2:  node->basicType = SpvBasicType::UInt64_2x2;  return;
		case SpvBasicType::UInt64_3:  node->basicType = SpvBasicType::UInt64_3x2;  return;
		case SpvBasicType::UInt64_4:  node->basicType = SpvBasicType::UInt64_4x2;  return;
		case SpvBasicType::Float16_2: node->basicType = SpvBasicType::Float16_2x2; return;
		case SpvBasicType::Float16_3: node->basicType = SpvBasicType::Float16_3x2; return;
		case SpvBasicType::Float16_4: node->basicType = SpvBasicType::Float16_4x2; return;
		case SpvBasicType::Float32_2: node->basicType = SpvBasicType::Float32_2x2; return;
		case SpvBasicType::Float32_3: node->basicType = SpvBasicType::Float32_3x2; return;
		case SpvBasicType::Float32_4: node->basicType = SpvBasicType::Float32_4x2; return;
		case SpvBasicType::Float64_2: node->basicType = SpvBasicType::Float64_2x2; return;
		case SpvBasicType::Float64_3: node->basicType = SpvBasicType::Float64_3x2; return;
		case SpvBasicType::Float64_4: node->basicType = SpvBasicType::Float64_4x2; return;
		} break;
	case 3:
		switch (comp_node->basicType) {
		case SpvBasicType::Bool_2:	  node->basicType = SpvBasicType::Bool_2x3;    return;
		case SpvBasicType::Bool_3:	  node->basicType = SpvBasicType::Bool_3x3;    return;
		case SpvBasicType::Bool_4:	  node->basicType = SpvBasicType::Bool_4x3;    return;
		case SpvBasicType::Int8_2:	  node->basicType = SpvBasicType::Int8_2x3;    return;
		case SpvBasicType::Int8_3:	  node->basicType = SpvBasicType::Int8_3x3;    return;
		case SpvBasicType::Int8_4:	  node->basicType = SpvBasicType::Int8_4x3;    return;
		case SpvBasicType::UInt8_2:	  node->basicType = SpvBasicType::UInt8_2x3;   return;
		case SpvBasicType::UInt8_3:	  node->basicType = SpvBasicType::UInt8_3x3;   return;
		case SpvBasicType::UInt8_4:	  node->basicType = SpvBasicType::UInt8_4x3;   return;
		case SpvBasicType::Int16_2:	  node->basicType = SpvBasicType::Int16_2x3;   return;
		case SpvBasicType::Int16_3:	  node->basicType = SpvBasicType::Int16_3x3;   return;
		case SpvBasicType::Int16_4:	  node->basicType = SpvBasicType::Int16_4x3;   return;
		case SpvBasicType::UInt16_2:  node->basicType = SpvBasicType::UInt16_2x3;  return;
		case SpvBasicType::UInt16_3:  node->basicType = SpvBasicType::UInt16_3x3;  return;
		case SpvBasicType::UInt16_4:  node->basicType = SpvBasicType::UInt16_4x3;  return;
		case SpvBasicType::Int32_2:	  node->basicType = SpvBasicType::Int32_2x3;   return;
		case SpvBasicType::Int32_3:	  node->basicType = SpvBasicType::Int32_3x3;   return;
		case SpvBasicType::Int32_4:	  node->basicType = SpvBasicType::Int32_4x3;   return;
		case SpvBasicType::UInt32_2:  node->basicType = SpvBasicType::UInt32_2x3;  return;
		case SpvBasicType::UInt32_3:  node->basicType = SpvBasicType::UInt32_3x3;  return;
		case SpvBasicType::UInt32_4:  node->basicType = SpvBasicType::UInt32_4x3;  return;
		case SpvBasicType::Int64_2:	  node->basicType = SpvBasicType::Int64_2x3;   return;
		case SpvBasicType::Int64_3:	  node->basicType = SpvBasicType::Int64_3x3;   return;
		case SpvBasicType::Int64_4:	  node->basicType = SpvBasicType::Int64_4x3;   return;
		case SpvBasicType::UInt64_2:  node->basicType = SpvBasicType::UInt64_2x3;  return;
		case SpvBasicType::UInt64_3:  node->basicType = SpvBasicType::UInt64_3x3;  return;
		case SpvBasicType::UInt64_4:  node->basicType = SpvBasicType::UInt64_4x3;  return;
		case SpvBasicType::Float16_2: node->basicType = SpvBasicType::Float16_2x3; return;
		case SpvBasicType::Float16_3: node->basicType = SpvBasicType::Float16_3x3; return;
		case SpvBasicType::Float16_4: node->basicType = SpvBasicType::Float16_4x3; return;
		case SpvBasicType::Float32_2: node->basicType = SpvBasicType::Float32_2x3; return;
		case SpvBasicType::Float32_3: node->basicType = SpvBasicType::Float32_3x3; return;
		case SpvBasicType::Float32_4: node->basicType = SpvBasicType::Float32_4x3; return;
		case SpvBasicType::Float64_2: node->basicType = SpvBasicType::Float64_2x3; return;
		case SpvBasicType::Float64_3: node->basicType = SpvBasicType::Float64_3x3; return;
		case SpvBasicType::Float64_4: node->basicType = SpvBasicType::Float64_4x3; return;
		} break;
	case 4:
		switch (comp_node->basicType) {
		case SpvBasicType::Bool_2:	  node->basicType = SpvBasicType::Bool_2x4;    return;
		case SpvBasicType::Bool_3:	  node->basicType = SpvBasicType::Bool_3x4;    return;
		case SpvBasicType::Bool_4:	  node->basicType = SpvBasicType::Bool_4x4;    return;
		case SpvBasicType::Int8_2:	  node->basicType = SpvBasicType::Int8_2x4;    return;
		case SpvBasicType::Int8_3:	  node->basicType = SpvBasicType::Int8_3x4;    return;
		case SpvBasicType::Int8_4:	  node->basicType = SpvBasicType::Int8_4x4;    return;
		case SpvBasicType::UInt8_2:	  node->basicType = SpvBasicType::UInt8_2x4;   return;
		case SpvBasicType::UInt8_3:	  node->basicType = SpvBasicType::UInt8_3x4;   return;
		case SpvBasicType::UInt8_4:	  node->basicType = SpvBasicType::UInt8_4x4;   return;
		case SpvBasicType::Int16_2:	  node->basicType = SpvBasicType::Int16_2x4;   return;
		case SpvBasicType::Int16_3:	  node->basicType = SpvBasicType::Int16_3x4;   return;
		case SpvBasicType::Int16_4:	  node->basicType = SpvBasicType::Int16_4x4;   return;
		case SpvBasicType::UInt16_2:  node->basicType = SpvBasicType::UInt16_2x4;  return;
		case SpvBasicType::UInt16_3:  node->basicType = SpvBasicType::UInt16_3x4;  return;
		case SpvBasicType::UInt16_4:  node->basicType = SpvBasicType::UInt16_4x4;  return;
		case SpvBasicType::Int32_2:	  node->basicType = SpvBasicType::Int32_2x4;   return;
		case SpvBasicType::Int32_3:	  node->basicType = SpvBasicType::Int32_3x4;   return;
		case SpvBasicType::Int32_4:	  node->basicType = SpvBasicType::Int32_4x4;   return;
		case SpvBasicType::UInt32_2:  node->basicType = SpvBasicType::UInt32_2x4;  return;
		case SpvBasicType::UInt32_3:  node->basicType = SpvBasicType::UInt32_3x4;  return;
		case SpvBasicType::UInt32_4:  node->basicType = SpvBasicType::UInt32_4x4;  return;
		case SpvBasicType::Int64_2:	  node->basicType = SpvBasicType::Int64_2x4;   return;
		case SpvBasicType::Int64_3:	  node->basicType = SpvBasicType::Int64_3x4;   return;
		case SpvBasicType::Int64_4:	  node->basicType = SpvBasicType::Int64_4x4;   return;
		case SpvBasicType::UInt64_2:  node->basicType = SpvBasicType::UInt64_2x4;  return;
		case SpvBasicType::UInt64_3:  node->basicType = SpvBasicType::UInt64_3x4;  return;
		case SpvBasicType::UInt64_4:  node->basicType = SpvBasicType::UInt64_4x4;  return;
		case SpvBasicType::Float16_2: node->basicType = SpvBasicType::Float16_2x4; return;
		case SpvBasicType::Float16_3: node->basicType = SpvBasicType::Float16_3x4; return;
		case SpvBasicType::Float16_4: node->basicType = SpvBasicType::Float16_4x4; return;
		case SpvBasicType::Float32_2: node->basicType = SpvBasicType::Float32_2x4; return;
		case SpvBasicType::Float32_3: node->basicType = SpvBasicType::Float32_3x4; return;
		case SpvBasicType::Float32_4: node->basicType = SpvBasicType::Float32_4x4; return;
		case SpvBasicType::Float64_2: node->basicType = SpvBasicType::Float64_2x4; return;
		case SpvBasicType::Float64_3: node->basicType = SpvBasicType::Float64_3x4; return;
		case SpvBasicType::Float64_4: node->basicType = SpvBasicType::Float64_4x4; return;
		} break;
	}
	
	throw Exception("unsupported matrix type");
}

void SpvParser::parseImageType(SpvImageTypeNode* node, spv_inst inst)
{
	node->elementTypeId = inst.get_id(2);
	node->dim           = inst.get_enum<spv::Dim>(3);
	node->depth         = inst.get_u32(4);
	node->arrayed       = inst.get_bool(5);
	node->multisampled  = inst.get_bool(6);
	node->sampled       = inst.get_u32(7);
	node->imageFormat   = inst.get_enum<spv::ImageFormat>(8);

	if (inst.length() > 9)
		node->accessQualifier = inst.get_enum<spv::AccessQualifier>(9);
	else
		node->accessQualifier = spv::AccessQualifierMax;
}

VERA_NAMESPACE_END
