/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2002-2021 PCSX2 Dev Team
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "GSMTLShaderCommon.h"

using namespace metal;

fragment float4 ps_interlace0(ConvertShaderData data [[stage_in]], ConvertPSRes res,
	constant GSMTLInterlacePSUniform& uniform [[buffer(GSMTLBufferIndexUniforms)]])
{
	if (fract(data.t.y * uniform.hH) - 0.5f < 0.f)
		discard_fragment();
	return res.sample(data.t);
}

fragment float4 ps_interlace1(ConvertShaderData data [[stage_in]], ConvertPSRes res,
	constant GSMTLInterlacePSUniform& uniform [[buffer(GSMTLBufferIndexUniforms)]])
{
	if (0.5f - fract(data.t.y * uniform.hH) < 0.f)
		discard_fragment();
	return res.sample(data.t);
}

fragment float4 ps_interlace2(ConvertShaderData data [[stage_in]], ConvertPSRes res,
	constant GSMTLInterlacePSUniform& uniform [[buffer(GSMTLBufferIndexUniforms)]])
{
	float4 c0 = res.sample(data.t - uniform.ZrH);
	float4 c1 = res.sample(data.t);
	float4 c2 = res.sample(data.t + uniform.ZrH);
	return (c0 + c1 * 2.f + c2) / 4.f;
}

fragment float4 ps_interlace3(ConvertShaderData data [[stage_in]], ConvertPSRes res)
{
	return res.sample(data.t);
}

