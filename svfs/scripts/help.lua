
Help = Help or { }
local HelpData = {}

local function DefaultHelp()
	print [[
	
StarVFS CLI
	
List of registered help commands:]]

	for k,v in pairs(HelpData) do
		print("", k)
	end

	print ""
	print "Type help 'XXX' to get about specific command"
	print ""
end

local function PrintHelp(info)
	print ""
	print(string.format("%s - %s", info.Command, info.Brief))
	print ""
	
	if info.Usage then
		print "Usage:"
		print(info.Usage)	
		print ""
	end
	
	
end

function help(what)	
	if not what or not HelpData[what] then
		DefaultHelp()
		return 
	end
	
	PrintHelp(HelpData[what])
end 

function Help.Register(info)
	if not info then
		print "Cannot register nil help!"
		return
	end
	
	if not info.Command or not info.Brief then
		print "Help info must have 'Command' and 'Brief' specified!"
		return
	end
	
	if HelpData[info.Command] then
		print "Cannot overwrite help info!"
		return 
	end
	
	HelpData[info.Command] = info
end
