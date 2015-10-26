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
        if l < 1 then return nil end -- Check for l < 1
        local s = "" -- Start string
        for i = 1, l do
                s = s .. string.char(math.random(97, 122)) -- Generate random number from 32 to 126, turn it into character and add to string
        end
        return s -- Return string
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


		elseif data.command=="getBig" then
			local retval={}
			serverDir="/var/www/"
			removingDelay=10
			symlinkPath=getFilenameFromPath(data.dir)--makeRandomString(64);
			-- symlinkPath=string.gsub(symlinkPath," "," ")
			if symlinkPath=="" then
				return "error"
			end
			io.popen("ln -s '"..data.dir.."' '"..serverDir..symlinkPath.."'\n".."chmod +rX '"..serverDir..symlinkPath.."'\n (sleep "..removingDelay.." ;rm '"..serverDir..symlinkPath.."')&")
			retval.data=symlinkPath
			retval.dir=data.dir

			retval.url="/"..symlinkPath

			retval.command="getBig"
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
