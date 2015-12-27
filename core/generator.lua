
local function generateDetectType(basedir, searchpattern, namespace, regfunc, outdata, addout)
	local files = { }
	for i,v in ipairs(os.matchfiles(basedir .. searchpattern)) do
		local n = string.match (v, "/([^i]%w+)%.h$")
		if n then
			local f = string.gsub(v, basedir, "", 1)
			files[#files + 1] = {
				file = f, 
				name = n,
				define = "STARVFS_DISABLE_" .. n,
			}
			outdata.headers[#outdata.headers + 1] =  {
				file = f,
				define = "STARVFS_DISABLE_" .. n,
			}
		end		
	end
	
	addout ""
	addout(string.format("namespace %s {", namespace)) 
	addout "\tinline"
	addout(string.format("	const char* Detect%sType(const String& Location) {", regfunc))  
	for i,v in ipairs(files) do
		addout(string.format("#ifndef %s", v.define:upper()))
		addout(string.format("\t\tif(%s::CanOpen(Location))", v.name))
		addout(string.format("\t\t\treturn \"%s\";", v.name))
		addout "#endif"
	end
	addout "\t}"
	addout ""
	addout "	template<class T> inline"
	addout(string.format("\tCreateContainerResult CreateContainer(const String& Location, const String& MountPoint, T* svfs) {"))  
	for i,v in ipairs(files) do
		addout(string.format("#ifndef %s", v.define:upper()))
		addout(string.format("\t\tif(%s::CanOpen(Location))", v.name))
		addout(string.format("\t\t\treturn %s::CreateFor(svfs, MountPoint, Location);", v.name))
		addout "#endif" --svfs->template CreateContainer<%s>(MountPoint, Location).second
	end
	addout "\t\treturn CreateContainerResult(VFSErrorCode::InternalError, nullptr);"
	addout "\t}"
	addout "}"
end

local function process(basedir, searchpattern, namespace, regfunc, outdata, addout)
	local files = { }
	for i,v in ipairs(os.matchfiles(basedir .. searchpattern)) do
		local n = string.match (v, "/([^i]%w+)%.h$")
		if n then
			local f = string.gsub(v, basedir, "", 1)
			files[#files + 1] = {
				file = f, 
				name = n,
				define = "STARVFS_DISABLE_" .. n,
			}
			outdata.headers[#outdata.headers + 1] =  {
				file = f,
				define = "STARVFS_DISABLE_" .. n,
			}
		end		
	end
	
	addout(string.format("namespace %s {", namespace)) 
	addout "\ttemplate<class T> inline"
	addout(string.format("	void Register%s(T &t) {", namespace))  
	for i,v in ipairs(files) do
		addout(string.format("#ifndef %s", v.define:upper()))
		addout(string.format("\t\tt.%s<%s>(\"%s\");", regfunc, v.name, v.name))
		addout "#endif"
	end
	addout "\t}"
	addout "}"
end

local function processExporters(basedir, outdata)
	local files = { }
	for i,v in ipairs(os.matchfiles(basedir .. "core/Exporter/*Exporter.h")) do
		local n = string.match (v, "/([^i]%w+)%.h$")
		if n then
			local f = string.gsub(v, basedir, "", 1)
			files[#files + 1] = {
				file = f, 
				name = n,
			}
			outdata.headers[#outdata.headers + 1] = f
			
		end		
	end
	
	local fbeg = [[
namespace Exporters {
	template<class T>
	void RegisterExporter(T t) {
]]
	local fm = ""
	for i,v in ipairs(files) do
		fm = fm .. string.format("\t\tt.RegisterExporter<%s>(\"%s\");\n", v.name, v.name)
	end
	local fend = [[
	}
}]]
	outdata.functions[#outdata.functions + 1] = fbeg .. fm .. fend
end

function GenerateModules(outfilename, basedir)
	basedir = basedir or ""
	
	if not outfilename then
		print "Out file name is not provided!"
		os.exit(1)
	end
	
	local outdata = {
		headers = { },
		functions = { 
			container = { },
			module = { },
			exporter = { },
		}
	}
	processExporters(basedir, outdata)
	local genout = { }

	local function addout(line, ...)
		if not line then
			return
		end
		genout[#genout + 1] = line
		addout(...)
	end
	
	local function moduleout(line, ...)
		if not line then
			return
		end
		local t = outdata.functions.module
		t[#t + 1] = line
		moduleout(...)
	end		
	
	local function containerout(line, ...)
		if not line then
			return
		end
		local t = outdata.functions.container
		t[#t + 1] = line
		containerout(...)
	end		
	
	local function exporterout(line, ...)
		if not line then
			return
		end
		local t = outdata.functions.exporter
		t[#t + 1] = line
		exporterout(...)
	end		
	
	process(basedir, "core/Container/*Container.h", "Containers", "RegisterContainer", outdata, containerout)
	process(basedir, "core/Module/*Module.h", "Modules", "RegisterModule", outdata, moduleout)
	process(basedir, "core/Exporter/*Exporter.h", "Exporters", "RegisterExporter", outdata, exporterout)	
	generateDetectType(basedir, "core/Container/*Container.h", "Containers", "Container", outdata, containerout)
	
	addout "/*"
	addout " * Automatically generated file"
	addout " * DO NOT EDIT!"
	addout " */"
	addout ""
	for i,v in ipairs(outdata.headers) do
		addout (string.format("#ifndef %s", v.define:upper()))
		addout (string.format("#include <%s>", v.file))
		addout "#endif"
	end
	addout ""
	addout "namespace StarVFS {"
	addout ""
	for i,v in pairs(outdata.functions) do
		for i,l in ipairs(v) do
			addout (l)
		end
		addout ""
	end
	
addout [[
template<class T>
void RegisterAll(T &t) {
	Containers::RegisterContainers(t);
	Modules::RegisterModules(t);
	Exporters::RegisterExporters(t);
}
]]	

	addout "}"
	
	f = io.open(outfilename, "w")
	for i,v in ipairs(genout) do
		f:write(v, "\n")
	end	
	f:close()
	
	print("Generated SVFS register " .. outfilename)
end