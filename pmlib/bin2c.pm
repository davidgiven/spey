-- $Id$
-- $Source$
-- $State$

local string_byte = string.byte
local string_len = string.len

local function bin2c_function(infile, outfile, symbol)
	local infp = io.open(infile, "rb")
	local outfp = io.open(outfile, "w")
	
	outfp:write("/* This is ", infile, " */\n")
	
	local data = infp:read("*a")
	local len = string_len(data)
	
	outfp:write("extern unsigned int ", symbol, "_length;\n")
	outfp:write("extern const char ", symbol, "_data[];\n")
	outfp:write("unsigned int ", symbol, "_length = ", len, ";\n")
	outfp:write("const char ", symbol, "_data[] = {\n")
	local count = 1
	for i = 1, len do
		outfp:write(string_byte(data, i), ", ")
		if (count == 16) then
			outfp:write("\n")
			count = 1
		else
			count = count + 1
		end
	end
	outfp:write("\n};\n")
	
	infp:close()
	outfp:close()
end

bin2c = simple {
	class = "bin2c",
	ensure_n_children = 1,

	outputs = {"%U%-%I%.c"},
	command = "dummy",
	
	__init = function(self, p)
 		simple.__init(self, p)
  		
		-- If we're a class, don't verify.
		
		if ((type(p) == "table") and p.class) then
			return
		end

		-- If symbol is not defined, bail.
		
		if (type(self.symbol) ~= "string") then
			self:__error("needs to have symbol defined, which should be a string")
		end
	end,

	__dobuild = function(self, inputs, outputs)
		local infile = inputs[1]
		local outfile = outputs[1]
		io.stderr:write("bin2c ", infile, " -> ", outfile, "\n")
		bin2c_function(infile, outfile, self:__expand(self.symbol))
	end
}

bin2cxx = bin2c {
	class = "bin2cxx",
	outputs = {"%U%-%I%.cc"},
}
