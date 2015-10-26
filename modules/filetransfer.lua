--
-- Copyright (C) 2015 Orange
--
-- This software is distributed under the terms and conditions of the 'Apache-2.0'
-- license which can be found in the file 'LICENSE' in this package distribution
-- or at 'http://www.apache.org/licenses/LICENSE-2.0'.
--
require ("os")
require ("lfs")
require ("json")
ltn12 = require ("ltn12")
mime = require ("mime")

function getDirFromPath(string)
	sep='/'
    return string:match("(.*"..sep..")")
end


function mysplit(inputstr, sep)
        if sep == nil then
                sep = "%s"
        end
        local t={} ; i=1
        for str in string.gmatch(inputstr, "([^"..sep.."]+)") do
                t[i] = str
                i = i + 1
        end
        return t
end


function getFilenameFromPath(string)
	sep='/'

	local data=mysplit(string,"/")

    return data[#data]
end

function makeRandomString(l)
        if l < 1 then return nil end 
        local s = ""
        for i = 1, l do
                s = s .. string.char(math.random(97, 122))
        end
        return s
end

function getDirectoryListing(path)
	local retdata={}
	retdata.command="list"
	local tab={}
	for file in lfs.dir(path) do
 			local object={}
 			object.type=lfs.attributes(path..'/'..file,"mode")
 			object.name=file
 			object.size=lfs.attributes (path..'/'..file, "size")
 			object.modtime=lfs.attributes (path..'/'..file, "modification")
 			table.insert(tab, object)
 		end
	retdata.list=tab
	retdata.path=path
 	return json.encode(retdata)
end


function init()
	return "filesystem line plugin init";
end

function onmessage(command)

	local retdata={}
 	data= json.decode(command)

 	if data.command=="list" then
 		return getDirectoryListing(data.dir)
    elseif data.command=="put" then

		ltn12.pump.all(
		  ltn12.source.string(data.data),
		  ltn12.sink.chain(
		    mime.decode("base64"),
		    ltn12.sink.file(io.open(data.dir,"w"))
		  )
		)
		return getDirectoryListing(getDirFromPath(data.dir))

    elseif data.command=="get" then
    	local t = {}
    	local retval={}
			ltn12.pump.all(
		  ltn12.source.file(io.open(data.dir,"r")),
			  ltn12.sink.chain(
			    mime.encode("base64"),
			    ltn12.sink.table(t)
			  	)
			)
			retval.data=table.concat(t)
			retval.dir=data.dir
			retval.command="get"
			return json.encode(retval)

    elseif data.command=="rename" then

			os.rename (data.dir, data.data)
			return getDirectoryListing(getDirFromPath(data.dir))

		elseif data.command=="copy" then
			if data.dir1 and data.dir2 then
				local iphandle =io.popen("mv '"..data.dir1.."' '"..data.dir2.."'");
				local data=iphandle:read("*a")
			end
			return getDirectoryListing(getDirFromPath(data.dir2))
		elseif data.command=="remove"then
    	os.remove (data.dir)
    	return getDirectoryListing(getDirFromPath(data.dir))
    end

		return json.encode(retdata)
end
