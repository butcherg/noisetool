#include <noise.h>
#include "noiseutils.h"
#include <stdlib.h>
#include <stdarg.h>

#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <limits>

std::vector<std::string> split(std::string s, std::string delim)
{
	std::vector<std::string> v;
	if (s.find(delim) == std::string::npos) {
		v.push_back(s);
		return v;
	}
	size_t pos=0;
	size_t start;
	while (pos < s.length()) {
		start = pos;
		pos = s.find(delim,pos);
		if (pos == std::string::npos) {
			v.push_back(s.substr(start,s.length()-start));
			return v;
		}
		v.push_back(s.substr(start, pos-start));
		pos += delim.length();
	}
	return v;
}

std::string string_format(const std::string fmt, ...) 
{
    int size = ((int)fmt.size()) * 2 + 50;   // Use a rubric appropriate for your code
    std::string str;
    va_list ap;
    while (1) {     // Maximum two passes on a POSIX system...
        str.resize(size);
        va_start(ap, fmt);
        int n = vsnprintf((char *)str.data(), size, fmt.c_str(), ap);
        va_end(ap);
        if (n > -1 && n < size) {  // Everything worked
            str.resize(n);
            return str;
        }
        if (n > -1)  // Needed size returned
            size = n + 1;   // For null char
        else
            size *= 2;      // Guess at a larger size (OS specific)
    }
    return str;
}

unsigned lineno;

void err(std::string msg)
{
	printf("Parse Error: %s.\n", msg.c_str());
	exit(EXIT_FAILURE);
}

void parse_err(std::string msg)
{
	printf("Error: %s. (line %d)\n", msg.c_str(), lineno);
	exit(EXIT_FAILURE);
}


using namespace noise;

std::string outputmodule, builder;

std::map<std::string, module::Module*> modules;
std::string dd;
std::string destfile;
int edgegradientdepth; float edgegradientelevation;

int destw = 80; int desth = 40;

float lowerXbound = 10.0;  float upperXbound = 15.0;  float lowerYbound = 10.0;  float upperYbound = 15.0;

void setBounds_XYWH(float x, float y, float w, float h)
{
	lowerXbound = x;  upperXbound = x+w;  lowerYbound = y;  upperYbound = y+h;
}

void setBounds_XXYY(float x1, float x2, float y1, float y2)
{
	lowerXbound = x1;  upperXbound = x2;  lowerYbound = y1;  upperYbound = y2;
}

void addDDNode(std::string name, std::string note)
{
		dd.append("\t"+name+" [label=<<TABLE BORDER=\"0\" CELLBORDER=\"1\" CELLSPACING=\"0\" CELLPADDING=\"2\"><TR><TD>"+name+"</TD></TR><TR><TD ALIGN=\"LEFT\"><FONT POINT-SIZE=\"6\">"+note+"</FONT></TD></TR></TABLE>>];\n");
}

void parseCache(std::string parms)
{
	std::string name, ddnote="module::Cache\n";
	module::Cache *s = new module::Cache();
	std::vector<std::string> pp =  split(std::string(parms), ";");
	for (std::vector<std::string>::iterator it = pp.begin(); it != pp.end(); ++it) {
		std::vector<std::string> parm =  split(*it, "=");
		if (parm[0].find("name") != std::string::npos) name = parm[1]; 
		else parse_err(string_format("Unrecognized keyword: %s", parm[0].c_str()));
	}
	modules[name] = s;
	addDDNode(name, ddnote);
}


//class generator Checkerboard
void parseCheckerboard(std::string parms)
{
	//printf("%s\n", parms.c_str());
	std::string name, ddnote="module::Checkerboard";
	module::Checkerboard *p = new module::Checkerboard();
	std::vector<std::string> pp =  split(std::string(parms), ";");
	for (std::vector<std::string>::iterator it = pp.begin(); it != pp.end(); ++it) {
		std::vector<std::string> parm =  split(*it, "=");
		if (parm[0].find("name") != std::string::npos) name = parm[1]; 
		else parse_err(string_format("Unrecognized keyword: %s", parm[0].c_str()));
	}
	modules[name] = p;
	addDDNode(name, ddnote);
}

//class generator Cylinders
void parseCylinders(std::string parms)
{
	//printf("%s\n", parms.c_str());
	std::string name, ddnote="module::Cylinders";
	module::Cylinders *p = new module::Cylinders();
	std::vector<std::string> pp =  split(std::string(parms), ";");
	for (std::vector<std::string>::iterator it = pp.begin(); it != pp.end(); ++it) {
		std::vector<std::string> parm =  split(*it, "=");
		if (parm[0].find("name") != std::string::npos) name = parm[1]; 
		else if (parm[0].find("freq") != std::string::npos) { p->SetFrequency(atof(parm[1].c_str())); ddnote.append("<BR/>frequency: "+parm[1]); }
		else parse_err(string_format("Unrecognized keyword: %s", parm[0].c_str()));
	}
	modules[name] = p;
	addDDNode(name, ddnote);
}

//class generator Perlin
void parsePerlin(std::string parms)
{
	//printf("%s\n", parms.c_str());
	std::string name, ddnote="module::Perlin\n";
	module::Perlin *p = new module::Perlin();
	std::vector<std::string> pp =  split(std::string(parms), ";");
	for (std::vector<std::string>::iterator it = pp.begin(); it != pp.end(); ++it) {
		std::vector<std::string> parm =  split(*it, "=");
		if (parm[0].find("name") != std::string::npos) name = parm[1]; 
		else if (parm[0].find("freq") != std::string::npos) { p->SetFrequency(atof(parm[1].c_str())); ddnote.append("<BR/>frequency: "+parm[1]); }
		else if (parm[0].find("lac") != std::string::npos)  { p->SetLacunarity(atof(parm[1].c_str())); ddnote.append("<BR/>lacunarity: "+parm[1]); }
		else if (parm[0].find("per") != std::string::npos)  { p->SetPersistence(atof(parm[1].c_str())); ddnote.append("<BR/>persistence: "+parm[1]); }
		else if (parm[0].find("oct") != std::string::npos)  { p->SetOctaveCount(atoi(parm[1].c_str())); ddnote.append("<BR/>octaves: "+parm[1]); }
		else if (parm[0].find("seed") != std::string::npos) { p->SetSeed(atoi(parm[1].c_str())); ddnote.append("<BR/>seed: "+parm[1]); }
		else parse_err(string_format("Unrecognized keyword: %s", parm[0].c_str()));
	}
	modules[name] = p;
	addDDNode(name, ddnote);
}

//class generator RidgedMulti
void parseRidgedMulti(std::string parms)
{
	std::string name, ddnote="module::RidgedMulti\n";
	module::RidgedMulti *p = new module::RidgedMulti();
	std::vector<std::string> pp =  split(std::string(parms), ";");
	for (std::vector<std::string>::iterator it = pp.begin(); it != pp.end(); ++it) {
		std::vector<std::string> parm =  split(*it, "=");
		if (parm[0].find("name") != std::string::npos) name = parm[1]; //else err("RidgedMulti module doesn't have a name");
		else if (parm[0].find("freq") != std::string::npos) { p->SetFrequency(atof(parm[1].c_str())); ddnote.append("<BR/>frequency: "+parm[1]); }
		else if (parm[0].find("lac") != std::string::npos)  { p->SetLacunarity(atof(parm[1].c_str())); ddnote.append("<BR/>lacunarity: "+parm[1]); }
		else if (parm[0].find("oct") != std::string::npos)  { p->SetOctaveCount(atoi(parm[1].c_str())); ddnote.append("<BR/>octaves: "+parm[1]); }
		else if (parm[0].find("seed") != std::string::npos) { p->SetSeed(atoi(parm[1].c_str())); ddnote.append("<BR/>seed: "+parm[1]); }
		else parse_err(string_format("Unrecognized keyword: %s", parm[0].c_str()));
	}
	modules[name] = p;
	addDDNode(name, ddnote);
}

//class generator Turbulence
void parseTurbulence(std::string parms)
{
	std::string name, ddnote="module::Turbulence\n";
	module::Turbulence *t = new module::Turbulence();
	std::vector<std::string> pp =  split(std::string(parms), ";");
	for (std::vector<std::string>::iterator it = pp.begin(); it != pp.end(); ++it) {
		std::vector<std::string> parm =  split(*it, "=");
		if (parm[0].find("name") != std::string::npos) name = parm[1]; //else err("RidgedMulti module doesn't have a name");
		else if (parm[0].find("freq") != std::string::npos) { t->SetFrequency(atof(parm[1].c_str())); ddnote.append("<BR/>frequency: "+parm[1]); }
		else if (parm[0].find("pow") != std::string::npos)  { t->SetPower(atof(parm[1].c_str())); ddnote.append("<BR/>power: "+parm[1]); }
		else if (parm[0].find("roug") != std::string::npos) { t->SetRoughness(atoi(parm[1].c_str()));  ddnote.append("<BR/>roughness: "+parm[1]); }
		else if (parm[0].find("seed") != std::string::npos) { t->SetSeed(atoi(parm[1].c_str())); ddnote.append("<BR/>seed: "+parm[1]); }
		else parse_err(string_format("Unrecognized keyword: %s", parm[0].c_str()));
	}
	modules[name] = t;
	addDDNode(name, ddnote);
}

//class generator Voronoi
void parseVoronoi(std::string parms)
{
	std::string name, ddnote="module::Voronoi\n";
	module::Voronoi *v = new module::Voronoi();
	std::vector<std::string> pp =  split(std::string(parms), ";");
	for (std::vector<std::string>::iterator it = pp.begin(); it != pp.end(); ++it) {
		std::vector<std::string> parm =  split(*it, "=");
		if (parm[0].find("name") != std::string::npos) name = parm[1];
		else if (parm[0].find("dist") != std::string::npos) { v->EnableDistance(true); ddnote.append("<BR/>distance: true"); }
		else if (parm[0].find("disp") != std::string::npos) { v->SetDisplacement(atof(parm[1].c_str())); ddnote.append("<BR/>displacement: "+parm[1]); }
		else if (parm[0].find("freq") != std::string::npos) { v->SetFrequency(atof(parm[1].c_str())); ddnote.append("<BR/>frequency: "+parm[1]); }
		else if (parm[0].find("seed") != std::string::npos) { v->SetSeed(atoi(parm[1].c_str())); ddnote.append("<BR/>seed: "+parm[1]); }
		else parse_err(string_format("Unrecognized keyword: %s", parm[0].c_str()));
	}
	modules[name] = v;
	addDDNode(name, ddnote);
}

//class generator Billow
void parseBillow(std::string parms)
{
	std::string name, ddnote="module::Billow\n";
	module::Billow *b = new module::Billow();
	std::vector<std::string> pp =  split(std::string(parms), ";");
	for (std::vector<std::string>::iterator it = pp.begin(); it != pp.end(); ++it) {
		std::vector<std::string> parm =  split(*it, "=");
		if (parm[0].find("name") != std::string::npos) name = parm[1];
		else if (parm[0].find("freq") != std::string::npos) { b->SetFrequency(atof(parm[1].c_str())); ddnote.append("<BR/>frequency: "+parm[1]); }
		else if (parm[0].find("lac") != std::string::npos)  { b->SetLacunarity(atof(parm[1].c_str())); ddnote.append("<BR/>lacunarity: "+parm[1]); }
		else if (parm[0].find("per") != std::string::npos)  { b->SetPersistence(atof(parm[1].c_str())); ddnote.append("<BR/>persistence: "+parm[1]); }
		else if (parm[0].find("oct") != std::string::npos)  { b->SetOctaveCount(atoi(parm[1].c_str())); ddnote.append("<BR/>octaves: "+parm[1]); }
		else if (parm[0].find("seed") != std::string::npos) { b->SetSeed(atoi(parm[1].c_str())); ddnote.append("<BR/>seed: "+parm[1]); }
		else parse_err(string_format("Unrecognized keyword: %s", parm[0].c_str()));
	}
	modules[name] = b;
	addDDNode(name, ddnote);
}

//class generator Const
void parseConst(std::string parms)
{
	std::string name, ddnote="module::Const\n";
	module::Const *s = new module::Const();
	std::vector<std::string> pp =  split(std::string(parms), ";");
	for (std::vector<std::string>::iterator it = pp.begin(); it != pp.end(); ++it) {
		std::vector<std::string> parm =  split(*it, "=");
		if (parm[0].find("name") != std::string::npos) name = parm[1]; 
		else if (parm[0].find("const") != std::string::npos) { s->SetConstValue(atoi(parm[1].c_str())); ddnote.append("<BR/>const: "+parm[1]); }
		else parse_err(string_format("Unrecognized keyword: %s", parm[0].c_str()));
	}
	modules[name] = s;
	addDDNode(name, ddnote);
}

//class aggregator Add
void parseAdd(std::string parms)
{
	std::string name, ddnote="module::Add\n";
	module::Add *s = new module::Add();
	std::vector<std::string> pp =  split(std::string(parms), ";");
	for (std::vector<std::string>::iterator it = pp.begin(); it != pp.end(); ++it) {
		std::vector<std::string> parm =  split(*it, "=");
		if (parm[0].find("name") != std::string::npos) name = parm[1]; 
		else parse_err(string_format("Unrecognized keyword: %s", parm[0].c_str()));
	}
	modules[name] = s;
	addDDNode(name, ddnote);
}

//class modifier Abs
void parseAbs(std::string parms)
{
	std::string name, ddnote="module::Abs\n";
	module::Abs *s = new module::Abs();
	std::vector<std::string> pp =  split(std::string(parms), ";");
	for (std::vector<std::string>::iterator it = pp.begin(); it != pp.end(); ++it) {
		std::vector<std::string> parm =  split(*it, "=");
		if (parm[0].find("name") != std::string::npos) name = parm[1]; 
		else parse_err(string_format("Unrecognized keyword: %s", parm[0].c_str()));
	}
	modules[name] = s;
	addDDNode(name, ddnote);
}

//class modifier Clamp
void parseClamp(std::string parms)
{
	std::string name, ddnote="module::Clamp\n";
	module::Clamp *s = new module::Clamp();
	std::vector<std::string> pp =  split(std::string(parms), ";");
	for (std::vector<std::string>::iterator it = pp.begin(); it != pp.end(); ++it) {
		std::vector<std::string> parm =  split(*it, "=");
		if (parm[0].find("name") != std::string::npos) name = parm[1];
		else if (parm[0].find("bounds") != std::string::npos) {
			std::vector<std::string> bb =  split(std::string(parm[1]), ",");
			s->SetBounds(atof(bb[0].c_str()), atof(bb[1].c_str())); 
			ddnote.append("<BR/>bounds: "+parm[1]); 
		}
		else parse_err(string_format("Unrecognized keyword: %s", parm[0].c_str()));
	}
	modules[name] = s;
	addDDNode(name, ddnote);
}

//class modifier Curve
void parseCurve(std::string parms)
{
	std::string name, ddnote="module::Curve\n";
	module::Curve *s = new module::Curve();
	std::vector<std::string> pp =  split(std::string(parms), ";");
	for (std::vector<std::string>::iterator it = pp.begin(); it != pp.end(); ++it) {
		std::vector<std::string> parm =  split(*it, "=");
		if (parm[0].find("name") != std::string::npos) name = parm[1];
		else if (parm[0].find("ctrlpoint") != std::string::npos) {
			std::vector<std::string> bb =  split(std::string(parm[1]), ",");
			s->AddControlPoint(atof(bb[0].c_str()), atof(bb[1].c_str())); 
			ddnote.append("<BR/>ctrlpoint: "+parm[1]); 
		}
		else parse_err(string_format("Unrecognized keyword: %s", parm[0].c_str()));
	}
	modules[name] = s;
	addDDNode(name, ddnote);
}

//class aggregator Blend
void parseBlend(std::string parms)
{
	std::string name, ddnote="module::Blend\n";
	module::Blend *s = new module::Blend();
	std::vector<std::string> pp =  split(std::string(parms), ";");
	for (std::vector<std::string>::iterator it = pp.begin(); it != pp.end(); ++it) {
		std::vector<std::string> parm =  split(*it, "=");
		if (parm[0].find("name") != std::string::npos) name = parm[1]; 
		else parse_err(string_format("Unrecognized keyword: %s", parm[0].c_str()));
	}
	modules[name] = s;
	addDDNode(name, ddnote);
}

//class aggregator Multiply
void parseMultiply(std::string parms)
{
	std::string name, ddnote="module::Multiply\n";
	module::Multiply *s = new module::Multiply();
	std::vector<std::string> pp =  split(std::string(parms), ";");
	for (std::vector<std::string>::iterator it = pp.begin(); it != pp.end(); ++it) {
		std::vector<std::string> parm =  split(*it, "=");
		if (parm[0].find("name") != std::string::npos) name = parm[1]; 
		else parse_err(string_format("Unrecognized keyword: %s", parm[0].c_str()));
	}
	modules[name] = s;
	addDDNode(name, ddnote);
}

//class aggregator Select
void parseSelect(std::string parms)
{
	std::string name, ddnote="module::Select\n";
	module::Select *s = new module::Select();
	std::vector<std::string> pp =  split(std::string(parms), ";");
	for (std::vector<std::string>::iterator it = pp.begin(); it != pp.end(); ++it) {
		std::vector<std::string> parm =  split(*it, "=");
		if (parm[0].find("name") != std::string::npos) name = parm[1]; //else err("Select module doesn't have a name");
		else if (parm[0].find("bounds") != std::string::npos) {
			std::vector<std::string> bb =  split(std::string(parm[1]), ",");
			s->SetBounds(atof(bb[0].c_str()), atof(bb[1].c_str())); 
			ddnote.append("<BR/>bounds: "+parm[1]); 
		}
		else if (parm[0].find("edge") != std::string::npos) { s->SetEdgeFalloff(atof(parm[1].c_str())); ddnote.append("<BR/>edgefalloff: "+parm[1]); }
		else parse_err(string_format("Unrecognized keyword: %s", parm[0].c_str()));
	}
	modules[name] = s;
	addDDNode(name, ddnote);
}

//class aggregator Max
void parseMax(std::string parms)
{
	std::string name, ddnote="module::Max\n";
	module::Max *s = new module::Max();
	std::vector<std::string> pp =  split(std::string(parms), ";");
	for (std::vector<std::string>::iterator it = pp.begin(); it != pp.end(); ++it) {
		std::vector<std::string> parm =  split(*it, "=");
		if (parm[0].find("name") != std::string::npos) name = parm[1]; 
		else parse_err(string_format("Unrecognized keyword: %s", parm[0].c_str()));
	}
	modules[name] = s;
	addDDNode(name, ddnote);
}

//class aggregator Min
void parseMin(std::string parms)
{
	std::string name, ddnote="module::Min\n";
	module::Min *s = new module::Min();
	std::vector<std::string> pp =  split(std::string(parms), ";");
	for (std::vector<std::string>::iterator it = pp.begin(); it != pp.end(); ++it) {
		std::vector<std::string> parm =  split(*it, "=");
		if (parm[0].find("name") != std::string::npos) name = parm[1]; 
		else parse_err(string_format("Unrecognized keyword: %s", parm[0].c_str()));
	}
	modules[name] = s;
	addDDNode(name, ddnote);
}

//class aggregator Power
void parsePower(std::string parms)
{
	std::string name, ddnote="module::Power\n";
	module::Power *s = new module::Power();
	std::vector<std::string> pp =  split(std::string(parms), ";");
	for (std::vector<std::string>::iterator it = pp.begin(); it != pp.end(); ++it) {
		std::vector<std::string> parm =  split(*it, "=");
		if (parm[0].find("name") != std::string::npos) name = parm[1]; 
		else parse_err(string_format("Unrecognized keyword: %s", parm[0].c_str()));
	}
	modules[name] = s;
	addDDNode(name, ddnote);
}

//class modifier ScaleBias
void parseScaleBias(std::string parms)
{
	std::string name, ddnote="module::ScaleBias\n";
	module::ScaleBias *s = new module::ScaleBias();
	std::vector<std::string> pp =  split(std::string(parms), ";");
	for (std::vector<std::string>::iterator it = pp.begin(); it != pp.end(); ++it) {
		std::vector<std::string> parm =  split(*it, "=");
		if (parm[0].find("name") != std::string::npos) name = parm[1]; 
		else if (parm[0].find("scale") != std::string::npos) { s->SetScale(atof(parm[1].c_str())); ddnote.append("<BR/>scale: "+parm[1]); }
		else if (parm[0].find("bias") != std::string::npos)  { s->SetBias(atof(parm[1].c_str())); ddnote.append("<BR/>bias: "+parm[1]); }
		else parse_err(string_format("Unrecognized keyword: %s", parm[0].c_str()));
	}
	modules[name] = s;
	addDDNode(name, ddnote);
}

//class modifier Displace
void parseDisplace(std::string parms)
{
	std::string name, ddnote="module::Displace\n";
	module::Displace *s = new module::Displace();
	std::vector<std::string> pp =  split(std::string(parms), ";");
	for (std::vector<std::string>::iterator it = pp.begin(); it != pp.end(); ++it) {
		std::vector<std::string> parm =  split(*it, "=");
		if (parm[0].find("name") != std::string::npos) name = parm[1]; 
		else parse_err(string_format("Unrecognized keyword: %s", parm[0].c_str()));
	}
	modules[name] = s;
	addDDNode(name, ddnote);
}

//class modifier Invert
void parseInvert(std::string parms)
{
	std::string name, ddnote="module::Invert\n";
	module::Invert *s = new module::Invert();
	std::vector<std::string> pp =  split(std::string(parms), ";");
	for (std::vector<std::string>::iterator it = pp.begin(); it != pp.end(); ++it) {
		std::vector<std::string> parm =  split(*it, "=");
		if (parm[0].find("name") != std::string::npos) name = parm[1]; 
		else parse_err(string_format("Unrecognized keyword: %s", parm[0].c_str()));
	}
	modules[name] = s;
	addDDNode(name, ddnote);
}


//class connector
void parseConnect(std::string parms)
{
	std::string source, sink;
	int instance = 0;
	if (parms.find("=") != std::string::npos) {
		std::vector<std::string> pp =  split(std::string(parms), ";");
		for (std::vector<std::string>::iterator it = pp.begin(); it != pp.end(); ++it) {
			std::vector<std::string> parm =  split(*it, "=");
			if (parm[0].find("source") != std::string::npos) source = parm[1]; 
			else if (parm[0].find("sink") != std::string::npos) sink = parm[1]; 
			else if (parm[0].find("inst") != std::string::npos) instance = atoi(parm[1].c_str()); 
			else parse_err(string_format("Unrecognized keyword: %s", parm[0].c_str()));
		}
	}
	else {
		std::vector<std::string> p =  split(std::string(parms), ",");
		if (p.size() < 3) parse_err("Not enough paramters for connect (source,sink,instance)");
		source = p[0];
		sink = p[1];
		instance = atoi(p[3].c_str());
		if (modules[sink]->GetSourceModuleCount() < instance) parse_err("This connect overloads the sink module connect count");
	}
	modules[sink]->SetSourceModule(instance, *(modules[source]));
	dd.append("\t"+source+" -> "+sink+";\n");
}

//class output
void parseOutput(std::string parms)
{
	std::string ddnote;
	std::vector<std::string> pp =  split(std::string(parms), ";");
	for (std::vector<std::string>::iterator it = pp.begin(); it != pp.end(); ++it) {
		std::vector<std::string> parm =  split(*it, "=");
		if (parm[0] == "module")  outputmodule = parm[1];
		//else if (parm[0] == "boundsxy") {
		//	std::vector<std::string> bb =  split(std::string(parm[1]), ",");
		//	if (bb.size() < 4) err("Bounds doesn't contain 4 numbers (x1,y1,x2,y2)");
		//	setBounds_XXYY(atof(bb[0].c_str()), atof(bb[1].c_str()), atof(bb[2].c_str()), atof(bb[3].c_str()));
		//}
		//if (parm[0].find("boundswh") != std::string::npos) {
		else if (parm[0] == "builder")  { builder = parm[1]; ddnote.append("builder: "+parm[1]+"<BR/>"); }
		else if (parm[0] == "bounds") {
			std::vector<std::string> bb =  split(std::string(parm[1]), ",");
			if (bb.size() < 4) err("Bounds doesn't contain 4 numbers (x,y,w,h)");
			setBounds_XYWH(atof(bb[0].c_str()), atof(bb[1].c_str()), atof(bb[2].c_str()), atof(bb[3].c_str()));
			ddnote.append("bounds: "+parm[1]+"<BR/>");
		}
		else if (parm[0] == "destfile") { destfile = parm[1]; ddnote.append("destfile: "+parm[1]+"<BR/>"); }
		else if (parm[0] == "destsize") {
			std::vector<std::string> dd =  split(std::string(parm[1]), ",");
			if (dd.size() < 2) err("Destsize doesn't contain 2 numbers (w,h)");
			destw = atoi(dd[0].c_str()); desth = atoi(dd[1].c_str());
			ddnote.append("destsize: "+parm[1]+"<BR/>"); 
		}
		else if (parm[0] == "edgegradientdepth") { edgegradientdepth = atoi(parm[1].c_str()); ddnote.append("edgegradientdepth: "+parm[1]+"<BR/>"); }
		else if (parm[0] == "edgegradientelevation") { edgegradientelevation = atoi(parm[1].c_str());  ddnote.append("edgegradientelevation: "+parm[1]+"<BR/>"); }
		else parse_err(string_format("Unrecognized keyword: %s", parm[0].c_str()));
	}
	if (outputmodule.size() == 0) parse_err("outputmodule parameter not defined");
	dd.append("\t"+outputmodule+" -> output\n");
	addDDNode("output", ddnote);
}


void parseFile(std::string filename)
{
	char line[256];
	std::ifstream netfile;
	netfile.open(filename, std::ifstream::in);
	if (!netfile.good()) err("File "+filename+" open not successful");
	for( std::string line; getline( netfile, line ); ) {
		lineno++;
		
		if (line[line.length()-1] == '\n') line.erase(line.length()-1);  //chomp line ending
		std::vector<std::string> ll =  split(std::string(line), "#");  //segregate comment (#)
		if (ll[0].empty()) continue;
		std::vector<std::string> l =  split(std::string(ll[0]), ":");
		
		//generators
		if (l[0] == "Billow") parseBillow(l[1]);
		else if (l[0] == "Cache") parseCache(l[1]);
		else if (l[0] == "Checkerboard") parseCheckerboard(l[1]);
		else if (l[0] == "Cylinders") parseCylinders(l[1]);
		else if (l[0] == "Perlin") parsePerlin(l[1]);
		else if (l[0] == "Ridgedmulti") parseRidgedMulti(l[1]);
		else if (l[0] == "Turbulence") parseTurbulence(l[1]);
		else if (l[0] == "Voronoi") parseVoronoi(l[1]);
		
		//aggregators:
		else if (l[0] == "Add") parseAdd(l[1]);
		else if (l[0] == "Abs") parseAbs(l[1]);
		else if (l[0] == "Blend") parseBlend(l[1]);
		else if (l[0] == "Max") parseMax(l[1]);
		else if (l[0] == "Min") parseMin(l[1]);
		else if (l[0] == "Multiply") parseMultiply(l[1]);
		else if (l[0] == "Power") parsePower(l[1]);
		else if (l[0] == "Select") parseSelect(l[1]);
		
		//modifiers:
		else if (l[0] == "Clamp") parseClamp(l[1]);
		else if (l[0] == "Const") parseConst(l[1]);
		else if (l[0] == "Curve") parseCurve(l[1]);
		else if (l[0] == "Displace") parseDisplace(l[1]);
		else if (l[0] == "Invert") parseInvert(l[1]);
		else if (l[0] == "ScaleBias") parseScaleBias(l[1]);
		
		//network:
		else if (l[0] == "Connect") parseConnect(l[1]);
		else if (l[0] == "Output") parseOutput(l[1]);
		else parse_err(string_format("Unrecognized keyword: %s",l[0].c_str()));
	}
	netfile.close();
}



int main(int argc, char **argv)
{	
	try {
	lineno=0;
	if (argc < 2) err("No network file specified.");
	
	fprintf(stderr, "parse network...\n"); fflush(stderr);
	dd.append("digraph noisetool {\n\trankdir=LR;\n\tnode [shape=plaintext];\n"); 
	parseFile(argv[1]);
	dd.append("}\n");
	
	if (outputmodule.size() == 0) err("No module connected to output pipeline");
	
	for (unsigned i=2; i<argc; i++) {
	
		if (std::string(argv[2]) == "digraph") {
			std::cout << dd; fflush(stdout);
			exit(EXIT_SUCCESS);
		}
	
		//the following read command line parameters to replace any set in the network file.  Same syntax
		if (std::string(argv[i]).find("bounds") != std::string::npos) {  
			std::vector<std::string> bb =  split(std::string(argv[i]), "=");
			if (bb.size() < 2) err("Malformed bounds (1)");
			std::vector<std::string> b = split(bb[1], ",");
			if (b.size() < 4) err("Malformed bounds (2)");
			setBounds_XYWH(atof(b[0].c_str()), atof(b[1].c_str()), atof(b[2].c_str()), atof(b[3].c_str()));
		}
		
		if (std::string(argv[i]).find("destsize") != std::string::npos) { 
			std::vector<std::string> dd =  split(std::string(argv[i]), "=");
			if (dd.size() < 2) err("Malformed destsize (destsize:w,h)");
			std::vector<std::string> d = split(dd[1], ",");
			if (d.size() < 2) err("Malformed destsize");
			destw = atoi(d[0].c_str());
			desth = atoi(d[1].c_str());
		}
		
		if (std::string(argv[i]).find("gradientdepth") != std::string::npos) {  
			std::vector<std::string> dd =  split(std::string(argv[i]), "=");
			if (dd.size() < 2) err("Malformed gradientdepth (gradientdepth:d)");
			edgegradientdepth = atoi(dd[1].c_str());
		}
		
		if (std::string(argv[i]).find("gradientelevation") != std::string::npos) {  
			std::vector<std::string> dd =  split(std::string(argv[i]), "=");
			if (dd.size() < 2) err("Malformed gradientelevation (gradientelevation:f)");
			edgegradientelevation = atoi(dd[1].c_str());
		}
		
		/*
		if (std::string(argv[i]).find("desttype") != std::string::npos) {  
			std::vector<std::string> d =  split(std::string(argv[i]), ":");
			if (d.size() < 2) err("Malformed desttype");
			desttype = d[1];
			if (desttype.find("bmp") == std::string::npos)
				if (desttype.find("ter") == std::string::npos)
					if (desttype.find("surface") == std::string::npos)
						err(string_format("Unknown destination file type: %s", desttype.c_str());
		}
		*/
		
		if (std::string(argv[i]).find("destfile") != std::string::npos) {  
			std::vector<std::string> d =  split(std::string(argv[i]), "=");
			if (d.size() < 2) err("Malformed destfile");
			destfile = d[1];
		}
	
	}
	
	//if (desttype.size() = 0) err("No destination type (desttype) defined on the command line");
	if (destfile.size() == 0) err("No destination filename (destfile) defined on the command line");
	
	
	
	printf("build noise map: "); fflush(stdout);
	utils::NoiseMap heightMap;
	
	if (builder == "cylinder") {
		printf("cylinder...\n"); fflush(stdout);
		err("Not implemented yet");
	}
	else if (builder == "plane") {
		printf("plane...\n"); fflush(stdout);
		utils::NoiseMapBuilderPlane heightMapBuilder;
		heightMapBuilder.SetSourceModule (*(modules[outputmodule]));
		heightMapBuilder.SetDestNoiseMap (heightMap);
		printf("\tdestination size: %d,%d\n", destw, desth); fflush(stdout);
		heightMapBuilder.SetDestSize (destw, desth);
		printf("\tbounds: %0.2f,%0.2f,%0.2f,%0.2f\n", lowerXbound, upperXbound, lowerYbound, upperYbound); fflush(stdout);
		heightMapBuilder.SetBounds (lowerXbound, upperXbound, lowerYbound, upperYbound);
		heightMapBuilder.Build ();
	}
	else if (builder == "sphere") {
		printf("sphere...\n"); fflush(stdout);
		err("Not implemented yet");
	}
	else if (builder == "brick") {
		printf("brick...\n"); fflush(stdout);
		utils::NoiseMapBuilderBrick heightMapBuilder;
		heightMapBuilder.SetSourceModule (*(modules[outputmodule]));
		heightMapBuilder.SetDestNoiseMap (heightMap);
		printf("\tdestination size: %d,%d\n", destw, desth); fflush(stdout);
		heightMapBuilder.SetDestSize (destw, desth);
		printf("\tbounds: %0.2f,%0.2f,%0.2f,%0.2f\n", lowerXbound, upperXbound, lowerYbound, upperYbound); fflush(stdout);
		heightMapBuilder.SetBounds (lowerXbound, upperXbound, lowerYbound, upperYbound);
		heightMapBuilder.Build ();
		heightMapBuilder.ApplyPositiveBias ();
		if (edgegradientdepth > 0 | edgegradientelevation > 0.0) {
			if (edgegradientdepth == 0) edgegradientdepth = 4;
			if (edgegradientelevation == 0.0) edgegradientelevation = 0.4;
			printf("\tedgegradientdepth: %d  edgegradientelevation: %f\n", edgegradientdepth, edgegradientelevation); fflush(stdout);
			heightMapBuilder.ApplyBorderGradient (edgegradientdepth, edgegradientelevation);
		}
	}
	
	float min = std::numeric_limits<float>::max(), max = std::numeric_limits<float>::min();
	
	for (unsigned y=0; y<heightMap.GetHeight(); y++) {
		for (unsigned x=0; x<heightMap.GetWidth(); x++) {
			float v = heightMap.GetValue(x,y);
			if (min > v) min = v;
			if (max < v) max = v;
		}
	}
	printf("\theightmap min: %f  max: %f\n",min,max); fflush(stdout);


	printf("generate output...\n"); fflush(stdout);
	
	std::vector<std::string> f =  split(destfile, ".");
	if (f.size() < 2) err("Destination filename doesn't contain a file extension");
	std::string ext = f[1];
	
	if (ext == "bmp") {  //source: image
		printf("\trender image...\n"); fflush(stdout);
		utils::RendererImage renderer;
		utils::Image image;
		renderer.SetSourceNoiseMap (heightMap);
		renderer.SetDestImage (image);
		renderer.Render ();
		
		printf("\twrite BMP image: %s...\n", destfile.c_str()); fflush(stdout);
		utils::WriterBMP writer;
		writer.SetSourceImage (image);
		writer.SetDestFilename (destfile);
		writer.WriteDestFile ();
	}
	else if (ext == "txt") {  //source: image, change to noisemap
		printf("\twrite TXT height image: %s...\n",destfile.c_str()); fflush(stdout);
		utils::WriterOpenSCADSurface writer;
		writer.SetSourceNoiseMap (heightMap);
		writer.SetDestFilename (destfile);
		writer.WriteDestFile ();
	}
	else if (ext == "ter") {  //source: noisemap
		printf("\twrite TER heightmap: %s...\n", destfile.c_str()); fflush(stdout);
		utils::WriterTER writer;
		writer.SetSourceNoiseMap (heightMap);
		writer.SetDestFilename (destfile);
		writer.WriteDestFile ();
	}
	
	printf("done.\n"); fflush(stdout);
	
	}
	catch (std::exception& ex ) {
		err(string_format("Invalid parameter: %s", ex.what()));
	}
	
	exit(EXIT_SUCCESS);

}