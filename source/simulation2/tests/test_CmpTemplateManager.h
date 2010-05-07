/* Copyright (C) 2010 Wildfire Games.
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

#include "lib/self_test.h"

#include "simulation2/system/ComponentManager.h"

#include "simulation2/components/ICmpTemplateManager.h"
#include "simulation2/components/ICmpTest.h"
#include "simulation2/MessageTypes.h"
#include "simulation2/system/ParamNode.h"
#include "simulation2/system/SimContext.h"
#include "simulation2/Simulation2.h"

#include "graphics/Terrain.h"
#include "ps/Filesystem.h"
#include "ps/CLogger.h"
#include "ps/XML/Xeromyces.h"

class TestCmpTemplateManager : public CxxTest::TestSuite
{
public:
	void setUp()
	{
		g_VFS = CreateVfs(20 * MiB);
		CXeromyces::Startup();
	}

	void tearDown()
	{
		CXeromyces::Terminate();
		g_VFS.reset();
	}

	void test_LoadTemplate()
	{
		TS_ASSERT_OK(g_VFS->Mount(L"", DataDir()/L"mods/_test.sim", VFS_MOUNT_MUST_EXIST));

		CSimContext context;
		CComponentManager man(context);
		man.LoadComponentTypes();

		entity_id_t ent1 = 1, ent2 = 2;
		CParamNode noParam;

		TS_ASSERT(man.AddComponent(ent1, CID_TemplateManager, noParam));

		ICmpTemplateManager* tempMan = static_cast<ICmpTemplateManager*> (man.QueryInterface(ent1, IID_TemplateManager));
		TS_ASSERT(tempMan != NULL);

		const CParamNode* basic = tempMan->LoadTemplate(ent2, L"basic", -1);
		TS_ASSERT(basic != NULL);
		TS_ASSERT_WSTR_EQUALS(basic->ToXML(), L"<Test1A>12345</Test1A>");

		const CParamNode* inherit2 = tempMan->LoadTemplate(ent2, L"inherit2", -1);
		TS_ASSERT(inherit2 != NULL);
		TS_ASSERT_WSTR_EQUALS(inherit2->ToXML(), L"<Test1A a=\"a2\" b=\"b1\" c=\"c1\"><d>d2</d><e>e1</e><f>f1</f><g>g2</g></Test1A>");

		const CParamNode* inherit1 = tempMan->LoadTemplate(ent2, L"inherit1", -1);
		TS_ASSERT(inherit1 != NULL);
		TS_ASSERT_WSTR_EQUALS(inherit1->ToXML(), L"<Test1A a=\"a1\" b=\"b1\" c=\"c1\"><d>d1</d><e>e1</e><f>f1</f></Test1A>");

		const CParamNode* actor = tempMan->LoadTemplate(ent2, L"actor|example1", -1);
		TS_ASSERT(actor != NULL);
		TS_ASSERT_WSTR_EQUALS(actor->ToXML(), L"<Position><Altitude>0</Altitude><Anchor>upright</Anchor><Floating>false</Floating></Position><VisualActor><Actor>example1</Actor></VisualActor>");

		const CParamNode* preview = tempMan->LoadTemplate(ent2, L"preview|unit", -1);
		TS_ASSERT(preview != NULL);
		TS_ASSERT_WSTR_EQUALS(preview->ToXML(), L"<Position><Altitude>0</Altitude><Anchor>upright</Anchor><Floating>false</Floating></Position><VisualActor><Actor>example</Actor></VisualActor>");

		const CParamNode* previewobstruct = tempMan->LoadTemplate(ent2, L"preview|unitobstruct", -1);
		TS_ASSERT(previewobstruct != NULL);
		TS_ASSERT_WSTR_EQUALS(previewobstruct->ToXML(), L"<Footprint><Circle radius=\"4\"></Circle><Height>1.0</Height></Footprint><Obstruction><Inactive></Inactive><Unit radius=\"4\"></Unit></Obstruction><Position><Altitude>0</Altitude><Anchor>upright</Anchor><Floating>false</Floating></Position><VisualActor><Actor>example</Actor></VisualActor>");

		const CParamNode* previewactor = tempMan->LoadTemplate(ent2, L"preview|actor|example2", -1);
		TS_ASSERT(previewactor != NULL);
		TS_ASSERT_WSTR_EQUALS(previewactor->ToXML(), L"<Position><Altitude>0</Altitude><Anchor>upright</Anchor><Floating>false</Floating></Position><VisualActor><Actor>example2</Actor></VisualActor>");
	}

	void test_LoadTemplate_errors()
	{
		TS_ASSERT_OK(g_VFS->Mount(L"", DataDir()/L"mods/_test.sim", VFS_MOUNT_MUST_EXIST));

		CSimContext context;
		CComponentManager man(context);
		man.LoadComponentTypes();

		entity_id_t ent1 = 1, ent2 = 2;
		CParamNode noParam;

		TS_ASSERT(man.AddComponent(ent1, CID_TemplateManager, noParam));

		ICmpTemplateManager* tempMan = static_cast<ICmpTemplateManager*> (man.QueryInterface(ent1, IID_TemplateManager));
		TS_ASSERT(tempMan != NULL);

		TestLogger logger;

		TS_ASSERT(tempMan->LoadTemplate(ent2, L"illformed", -1) == NULL);

		TS_ASSERT(tempMan->LoadTemplate(ent2, L"invalid", -1) == NULL);

		TS_ASSERT(tempMan->LoadTemplate(ent2, L"nonexistent", -1) == NULL);

		TS_ASSERT(tempMan->LoadTemplate(ent2, L"inherit-loop", -1) == NULL);

		TS_ASSERT(tempMan->LoadTemplate(ent2, L"inherit-broken", -1) == NULL);

		TS_ASSERT(tempMan->LoadTemplate(ent2, L"inherit-special", -1) == NULL);

		TS_ASSERT(tempMan->LoadTemplate(ent2, L"preview|nonexistent", -1) == NULL);
	}

	void test_LoadTemplate_multiple()
	{
		TS_ASSERT_OK(g_VFS->Mount(L"", DataDir()/L"mods/_test.sim", VFS_MOUNT_MUST_EXIST));

		CSimContext context;
		CComponentManager man(context);
		man.LoadComponentTypes();

		entity_id_t ent1 = 1, ent2 = 2;
		CParamNode noParam;

		TS_ASSERT(man.AddComponent(ent1, CID_TemplateManager, noParam));

		ICmpTemplateManager* tempMan = static_cast<ICmpTemplateManager*> (man.QueryInterface(ent1, IID_TemplateManager));
		TS_ASSERT(tempMan != NULL);

		const CParamNode* basicA = tempMan->LoadTemplate(ent2, L"basic", -1);
		TS_ASSERT(basicA != NULL);

		const CParamNode* basicB = tempMan->LoadTemplate(ent2, L"basic", -1);
		TS_ASSERT(basicA == basicB);

		const CParamNode* inherit2A = tempMan->LoadTemplate(ent2, L"inherit2", -1);
		TS_ASSERT(inherit2A != NULL);

		const CParamNode* inherit2B = tempMan->LoadTemplate(ent2, L"inherit2", -1);
		TS_ASSERT(inherit2A == inherit2B);

		TestLogger logger;

		TS_ASSERT(tempMan->LoadTemplate(ent2, L"nonexistent", -1) == NULL);
		TS_ASSERT(tempMan->LoadTemplate(ent2, L"nonexistent", -1) == NULL);

		TS_ASSERT(tempMan->LoadTemplate(ent2, L"inherit-loop", -1) == NULL);
		TS_ASSERT(tempMan->LoadTemplate(ent2, L"inherit-loop", -1) == NULL);

		TS_ASSERT(tempMan->LoadTemplate(ent2, L"inherit-broken", -1) == NULL);
		TS_ASSERT(tempMan->LoadTemplate(ent2, L"inherit-broken", -1) == NULL);
	}

	void test_load_all_DISABLED() // disabled since it's a bit slow
	{
		TS_ASSERT_OK(g_VFS->Mount(L"", DataDir()/L"mods/public", VFS_MOUNT_MUST_EXIST));

		CTerrain dummy;
		CSimulation2 sim(NULL, &dummy);
		sim.LoadDefaultScripts();
		sim.ResetState();

		CmpPtr<ICmpTemplateManager> cmpTempMan(sim, SYSTEM_ENTITY);
		TS_ASSERT(!cmpTempMan.null());

		std::vector<std::wstring> templates = cmpTempMan->FindAllTemplates();
		for (size_t i = 0; i < templates.size(); ++i)
		{
			std::wstring name = templates[i];
			printf("# %ls\n", name.c_str());
			const CParamNode* p = cmpTempMan->GetTemplate(name);
			TS_ASSERT(p != NULL);
		}
	}
};
