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

#include "MessageHandler.h"
#include "../GameLoop.h"
#include "../CommandProc.h"

#include "graphics/GameView.h"
#include "graphics/LOSTexture.h"
#include "graphics/MapWriter.h"
#include "graphics/Patch.h"
#include "graphics/Terrain.h"
#include "graphics/TerrainTextureEntry.h"
#include "graphics/TerrainTextureManager.h"
#include "ps/Game.h"
#include "ps/Loader.h"
#include "ps/World.h"
#include "renderer/Renderer.h"
#include "scriptinterface/ScriptInterface.h"
#include "simulation2/Simulation2.h"
#include "simulation2/components/ICmpPlayer.h"
#include "simulation2/components/ICmpPlayerManager.h"
#include "simulation2/components/ICmpPosition.h"
#include "simulation2/components/ICmpRangeManager.h"
#include "simulation2/components/ICmpTerrain.h"

namespace
{
	void InitGame()
	{
		if (g_Game)
		{
			delete g_Game;
			g_Game = NULL;
		}

		g_Game = new CGame();

		// Default to player 1 for playtesting
		g_Game->SetPlayerID(1);
	}

	void StartGame(const CScriptValRooted& attrs)
	{
		g_Game->StartGame(attrs);

		// TODO: Non progressive load can fail - need a decent way to handle this
		LDR_NonprogressiveLoad();
		
		PSRETURN ret = g_Game->ReallyStartGame();
		ENSURE(ret == PSRETURN_OK);

		// Disable fog-of-war
		CmpPtr<ICmpRangeManager> cmpRangeManager(*g_Game->GetSimulation2(), SYSTEM_ENTITY);
		if (!cmpRangeManager.null())
			cmpRangeManager->SetLosRevealAll(-1, true);
	}
}

namespace AtlasMessage {

QUERYHANDLER(GenerateMap)
{
	InitGame();

	// Random map
	ScriptInterface& scriptInterface = g_Game->GetSimulation2()->GetScriptInterface();
	
	CScriptValRooted settings = scriptInterface.ParseJSON(*msg->settings);

	CScriptValRooted attrs;
	scriptInterface.Eval("({})", attrs);
	scriptInterface.SetProperty(attrs.get(), "mapType", std::string("random"));
	scriptInterface.SetProperty(attrs.get(), "script", std::wstring(*msg->filename));
	scriptInterface.SetProperty(attrs.get(), "settings", settings, false);

	try
	{
		StartGame(attrs);

		msg->status = 0;
	}
	catch (PSERROR_Game_World_MapLoadFailed e)
	{
		// Cancel loading
		LDR_Cancel();

		msg->status = -1;
	}
}

MESSAGEHANDLER(LoadMap)
{
	InitGame();

	// Scenario
	CStrW map = *msg->filename;
	CStrW mapBase = map.BeforeLast(L".pmp"); // strip the file extension, if any

	ScriptInterface& scriptInterface = g_Game->GetSimulation2()->GetScriptInterface();
	
	CScriptValRooted attrs;
	scriptInterface.Eval("({})", attrs);
	scriptInterface.SetProperty(attrs.get(), "mapType", std::string("scenario"));
	scriptInterface.SetProperty(attrs.get(), "map", std::wstring(mapBase));

	StartGame(attrs);
}

MESSAGEHANDLER(SaveMap)
{
	CMapWriter writer;
	const VfsPath pathname = VfsPath("maps/scenarios") / *msg->filename;
	writer.SaveMap(pathname,
		g_Game->GetWorld()->GetTerrain(),
		g_Renderer.GetWaterManager(), g_Renderer.GetSkyManager(),
		&g_LightEnv, g_Game->GetView()->GetCamera(), g_Game->GetView()->GetCinema(),
		g_Game->GetSimulation2());
}

QUERYHANDLER(GetMapSettings)
{
	msg->settings = g_Game->GetSimulation2()->GetMapSettingsString();
}

QUERYHANDLER(GetMapSizes)
{
	msg->sizes = g_Game->GetSimulation2()->GetMapSizes();
}

BEGIN_COMMAND(SetMapSettings)
{
	void Do()
	{
		Redo();
	}

	void Undo()
	{
		// TODO
		debug_warn(L"Can't undo SetMapSettings");
	}

	void Redo()
	{
		g_Game->GetSimulation2()->SetMapSettings(*msg->settings);
	}
};
END_COMMAND(SetMapSettings)

QUERYHANDLER(GetRMSData)
{
	msg->data = g_Game->GetSimulation2()->GetRMSData();
}

BEGIN_COMMAND(ResizeMap)
{
	cResizeMap()
	{
	}

	void MakeDirty()
	{
		CmpPtr<ICmpTerrain> cmpTerrain(*g_Game->GetSimulation2(), SYSTEM_ENTITY);
		if (!cmpTerrain.null())
			cmpTerrain->ReloadTerrain();

		// The LOS texture won't normally get updated when running Atlas
		// (since there's no simulation updates), so explicitly dirty it
		g_Game->GetView()->GetLOSTexture().MakeDirty();
	}

	void Do()
	{
		Redo();
	}

	void Undo()
	{
		// TODO
		debug_warn(L"Can't undo ResizeMap");
	}

	void Redo()
	{
		CTerrain* terrain = g_Game->GetWorld()->GetTerrain();

		terrain->Resize(msg->tiles / PATCH_SIZE);

		MakeDirty();
	}
};
END_COMMAND(ResizeMap)

}
