#pragma once

#include "vector_base.h"
#include "config.h"

VERA_NAMESPACE_BEGIN

#ifdef VERA_MATRIX_GL_LAYOUT

template <MathDimType Col, MathDimType Row, class T, MathQualifier Q>
class matrix_base
{
	static_assert("unsupported matrix type");
};

#else

template <MathDimType Row, MathDimType Col, class T, MathQualifier Q>
class matrix_base
{
	static_assert("unsupported matrix type");
};

#endif

VERA_NAMESPACE_END
