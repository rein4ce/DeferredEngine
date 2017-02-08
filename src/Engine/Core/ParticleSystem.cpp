#include "platform.h"
#include "ParticleSystem.h"
#include "Engine.h"
#include "Scene.h"
#include <boost/regex.hpp>
#include <boost\algorithm\string\case_conv.hpp>
#include "Game.h"
#include "Effect.h"
#include "ConsolePanel.h"

CParticleEffectsManager gEffects;


CParticleSystem::CParticleSystem(CScene *pScene, int maxParticles)
{
	this->pScene = pScene;
	Init(maxParticles);
}

CParticleSystem::~CParticleSystem()
{
	Reset();
}

//////////////////////////////////////////////////////////////////////////
void CParticleSystem::Reset()
{
	particles.Release();
}

//////////////////////////////////////////////////////////////////////////
void CParticleSystem::Init(int maxParticles)
{
	Reset();
	particles.Init(maxParticles);
}

//////////////////////////////////////////////////////////////////////////
void CParticleSystem::Add( CParticle *particle )
{
	int slot = particles.Add(particle);
	if (slot == -1) 
	{
		SAFE_DELETE(particle);
		return;
	}

	pScene->AddSprite((CSprite*)particle);

	particle->id = slot;
	particle->killtime = gEngine.realtime + particle->lifetime;
	if (particle->colorTime == 0) particle->colorTime = particle->lifetime;				// if colorTime not set, use lifetime
	particle->colorEnd = gEngine.realtime + particle->colorTime;

	// calculate color step
	particle->colorStep[0] = (sint16)particle->colorChange.r - particle->color.r;
	particle->colorStep[1] = (sint16)particle->colorChange.g - particle->color.g;
	particle->colorStep[2] = (sint16)particle->colorChange.b - particle->color.b;
	particle->colorStep[3] = (sint16)particle->colorChange.a - particle->color.a;
}

//////////////////////////////////////////////////////////////////////////
void CParticleSystem::Remove( CParticle *particle )
{
	pScene->RemoveSprite((CSprite*)particle);
	particles.Remove(particle->id);
	SAFE_DELETE(particle);
}

//////////////////////////////////////////////////////////////////////////
void CParticleSystem::Update( float frametime, float realtime )
{
	int updated = 0;
	int r,g,b,a;

	for (int i=0; i<particles.maxElements; i++)
	{
		if (particles[i] == NULL) continue;
		CParticle *p = particles[i];

		// check if particle should die
		if (p->deleteMe || (p->lifetime != 0 && p->killtime <= realtime))
		{
			//Debug("Sprite %d died", i);
			pScene->RemoveSprite((CSprite*)p);
			particles.Remove(i);
			continue;
		}

		// speed, position
		p->velocity += p->acceleration * frametime;
		p->velocity.y -= p->gravity * frametime;
		p->position += p->velocity * frametime;

		// size, rotation
		p->size += p->sizeVel * frametime;
		p->rotation += p->rotationVel * frametime;

		// color, alpha
		if (p->colorTime > 0 && p->colorEnd > realtime)
		{
			r = (int)p->color.r + (float)p->colorStep[0] * frametime / p->colorTime;
			g = (int)p->color.g + (float)p->colorStep[1] * frametime / p->colorTime;
			b = (int)p->color.b + (float)p->colorStep[2] * frametime / p->colorTime;
			a = (int)p->color.a + (float)p->colorStep[3] * frametime / p->colorTime;

			p->color.r = clamp(r, 0, 255);
			p->color.g = clamp(g, 0, 255);
			p->color.b = clamp(b, 0, 255);
			p->color.a = clamp(a, 0, 255);
		}

		// additional callback
		if (p->updateCallback)
			p->updateCallback(p);
		
		if (++updated >= particles.count) break;
	}

	//PrintText(10, 300, "Particles: %d", particles.count);
}

//////////////////////////////////////////////////////////////////////////
void CParticleSystem::Render( float frametime, float realtime )
{
	
}

//////////////////////////////////////////////////////////////////////////
CParticle::CParticle(CTexture *tex, CObject3D *pParent)
{
	rotationVel = 0; 
	lifetime = 0;	
	killtime = 0;	
	pParent = NULL;
	updateCallback = NULL;
	gravity = 0;
	colorTime = 0;
	deleteMe = false;

	this->pTexture = tex;
	this->pParent = pParent;
}

//////////////////////////////////////////////////////////////////////////
// Load effects from file
//////////////////////////////////////////////////////////////////////////
void CParticleEffectsManager::Init()
{
	Release();

	ifstream is("particles/effects.txt");	
	
	boost::match_results<std::string::const_iterator> what;
	boost::regex reEffect("^#\\s*(.+)$");			// new effect
	boost::regex reParticle("^\\+(\\s*(.+))?$");		// new particle
	boost::regex reProperty("^(\\w+)\\s+(\\S+)(\\s(\\S+))?(\\s(\\S+))?(\\s(\\S+))?$");		// particle properties
	string line;

	CArray<CParticleEffect*> effects;
	CParticleEffect *effect = NULL;
	SParticleEffectInfo *particle = NULL;

	while (std::getline(is, line))
	{	
		if (effects.Size() > 0) 
			effect = effects[effects.Size()-1];

		if (effect && effect->particles.Size() > 0)
			particle = &effect->particles[effect->particles.Size()-1];


		// check if it's a new effect line
		if (line[0] == '#' && boost::regex_match(line, what, reEffect))
		{
			CParticleEffect *e = new CParticleEffect();
			SParticleEffectInfo p;			
			e->name = string(what[1].first, what[1].second);
			effects.Add(e);		
			effects[effects.Size()-1]->particles = CArray<SParticleEffectInfo>();
			effects[effects.Size()-1]->particles.Add(p);
			Debug("New effect: %s", e->name.c_str());
			continue;
		}

		// check if it's a new effect 
		if (line[0] == '+' && boost::regex_match(line, what, reParticle))
		{
			SParticleEffectInfo p;
			if (what.size() > 1) p.time = atoi(string(what[1].first, what[1].second).c_str());
			effect->particles.Add(p);
			Debug("New particle with time %f", p.time);
			continue;
		}

		// check properties
		if (boost::regex_match(line, what, reProperty))
		{
			string name = boost::to_lower_copy(string(what[1].first, what[1].second));
			string values[4] = { string(what[2].first, what[2].second), "", "", "" };

			if (what.size() > 4) values[1] = string(what[4].first, what[4].second);
			if (what.size() > 5) values[2] = string(what[5].first, what[5].second);
			if (what.size() > 6) values[3] = string(what[6].first, what[6].second);

			if (name == "texture")		
			{
				particle->texture = values[0];
			}
			else if (name == "position")	
			{
				particle->posX = ParseRangeValue(values[0]);
				particle->posY = ParseRangeValue(values[1]);
				particle->posZ = ParseRangeValue(values[2]);
			}
			else if (name == "size")
			{
				particle->sizeX = ParseRangeValue(values[0]);
				particle->sizeY = ParseRangeValue(values[1]);
			}
			else if (name == "sizevel")
			{
				particle->sizeVelX = ParseRangeValue(values[0]);
				particle->sizeVelY = ParseRangeValue(values[1]);
			}
			else if (name == "count")
			{
				particle->count = ParseRangeValue(values[0]);				
			}
			else if (name == "velocity")
			{
				particle->velX = ParseRangeValue(values[0]);
				particle->velY = ParseRangeValue(values[1]);
				particle->velZ = ParseRangeValue(values[2]);
			}
			else if (name == "acceleration")
			{
				particle->accX = ParseRangeValue(values[0]);
				particle->accY = ParseRangeValue(values[1]);
				particle->accZ = ParseRangeValue(values[2]);
			}
			else if (name == "rotation")
			{
				particle->rotation = ParseRangeValue(values[0]);
			}
			else if (name == "rotationVel")
			{
				particle->rotationVel = ParseRangeValue(values[0]);
			}
			else if (name == "color")
			{
				particle->r = ParseRangeValue(values[0]);
				particle->g = ParseRangeValue(values[1]);
				particle->b = ParseRangeValue(values[2]);
				particle->a = ParseRangeValue(values[3]);
			}
			else if (name == "colorend")
			{
				particle->er = ParseRangeValue(values[0]);
				particle->eg = ParseRangeValue(values[1]);
				particle->eb = ParseRangeValue(values[2]);
				particle->ea = ParseRangeValue(values[3]);
			}
			else if (name == "lifetime")
			{
				particle->lifetime = ParseRangeValue(values[0]);
			}
			else if (name == "additive")
			{
				particle->additive = values[0] == "true" ? true : false;
			}
		}
	}

	Debug("Loaded %d effects", effects.Size());

	// Now let's put the effects in to the map
	for (int i=0; i<effects.Size(); i++)
	{
		this->effects[effects[i]->name] = effects[i];
	}
}

//////////////////////////////////////////////////////////////////////////
void CParticleEffectsManager::Update()
{
	if (HasFileChanged("particles/effects.txt"))
	{
		CONSOLE("Reloading effects");
		Init();
	}
}

//////////////////////////////////////////////////////////////////////////
SRange CParticleEffectsManager::ParseRangeValue( string value )
{
	boost::match_results<std::string::const_iterator> what;
	boost::regex re("^rand\\((\\S+),(\\S+)\\)");

	if (boost::regex_match(value, what, re))
	{
		string v1 = string(what[1].first, what[1].second);
		string v2 = string(what[2].first, what[2].second);
		return SRange(
			atof(v1.c_str()),
			atof(v2.c_str())
			);
	}
	else
		return SRange(atof(value.c_str()));
}

//////////////////////////////////////////////////////////////////////////
void CParticleEffectsManager::Release()
{
	map<string, CParticleEffect*>::iterator iter;
	for (iter=effects.begin(); iter!=effects.end(); iter++)
		SAFE_DELETE(iter->second);
	effects.clear();
}

//////////////////////////////////////////////////////////////////////////
CParticleEffect* CParticleEffectsManager::Get( string name )
{
	map<string, CParticleEffect*>::iterator iter = effects.find(name);
	return iter == effects.end() ? NULL : iter->second;
}

//////////////////////////////////////////////////////////////////////////
CEntity* CParticleEffectsManager::Create( string name, Vector3 pos )
{
	CParticleEffect *e = Get(name);
	Assert(e);
	CEffect *ent = new CEffect();
	ent->pEffect = e;
	ent->SetPosition(pos);
	pGame->AddEntity(ent);
	return ent;
}

//////////////////////////////////////////////////////////////////////////
void SParticleEffectInfo::Spawn( Vector3 pos )
{
	int total = count.Get();
	for (int i=0; i<total; i++)
	{
		CParticle *p = new CParticle(CTexture::Get(texture));
		p->position = pos;
		p->position.x += posX.Get();
		p->position.y += posY.Get();
		p->position.z += posZ.Get();

		p->velocity.x = velX.Get();
		p->velocity.y = velY.Get();
		p->velocity.z = velZ.Get();

		p->acceleration.x = accX.Get();
		p->acceleration.y = accY.Get();
		p->acceleration.z = accZ.Get();

		p->size.x = sizeX.Get();
		p->size.y = sizeY.Get();

		p->sizeVel.x = sizeVelX.Get();
		p->sizeVel.y = sizeVelY.Get();

		p->rotation = rotation.Get();
		p->rotationVel = rotationVel.Get();

		p->color.r = (byte)r.Get();
		p->color.g = (byte)g.Get();
		p->color.b = (byte)b.Get();
		p->color.a = (byte)a.Get();

		p->colorChange.r = (byte)er.Get();
		p->colorChange.g = (byte)eg.Get();
		p->colorChange.b = (byte)eb.Get();
		p->colorChange.a = (byte)ea.Get();

		p->lifetime = lifetime.Get();
		p->additive = additive;

		p->depthWrite = false;

		pGame->particles->Add(p);
	}
}