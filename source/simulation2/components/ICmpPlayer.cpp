/* Copyright (C) 2011 Wildfire Games.
 * This file is part of 0 A.D.
 *
 * 0 A.D. is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * 0 A.D. is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with 0 A.D.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "precompiled.h"

#include "ICmpPlayer.h"

#include "maths/FixedVector3D.h"
#include "simulation2/system/InterfaceScripted.h"
#include "simulation2/scripting/ScriptComponent.h"

#include "ps/Overlay.h"

BEGIN_INTERFACE_WRAPPER(Player)
END_INTERFACE_WRAPPER(Player)

class CCmpPlayerScripted : public ICmpPlayer
{
public:
	DEFAULT_SCRIPT_WRAPPER(PlayerScripted)

	virtual void SetName(const std::wstring& name)
	{
		m_Script.CallVoid("SetName", name);
	}

	virtual void SetCiv(const std::wstring& civcode)
	{
		m_Script.CallVoid("SetCiv", civcode);
	}

	virtual void SetColour(u8 r, u8 g, u8 b)
	{
		m_Script.CallVoid("SetColour", (u32)r, (u32)g, (u32)b);
	}

	virtual CColor GetColour()
	{
		return m_Script.Call<CColor>("GetColour");
	}

	virtual CFixedVector3D GetStartingCameraPos()
	{
		return m_Script.Call<CFixedVector3D>("GetStartingCameraPos");
	}

	virtual CFixedVector3D GetStartingCameraRot()
	{
		return m_Script.Call<CFixedVector3D>("GetStartingCameraRot");
	}

	virtual bool HasStartingCamera()
	{
		return m_Script.Call<bool>("HasStartingCamera");
	}
};

REGISTER_COMPONENT_SCRIPT_WRAPPER(PlayerScripted)
