require ("os")
require ("json")
require ("lfs")

local debug=true
local streamingScreen="streamZVideo"
local streamingDataFile="streamingConfig"
--    ffmpeg -f video4linux2 -framerate 25 -video_size  320x180 -i /dev/video0
--    -f flv "rtmp://publish.dailymotion.com/publish-dm/x2pa4an?auth=0IFa_abbca8d62709ad65b2564161ebfd27670aba74f1"


function startScreenCommand(screenname,command)

	io.popen("screen -dmS "..screenname.." "..command.."\n")
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

function stopScreen(screenname)

	io.popen("screen -X -S "..screenname.." quit\n")
end

function getDevices()
	local tab={}
	for file in lfs.dir("/dev") do
		if string.match(file,"video") then
 			table.insert(tab,"/dev/"..file)
		end
 	end

--	table.insert(tab,"Webcam1")

	if debug then
		table.insert(tab,"test.mp4")
	end

	local retval={}
		retval.devices=tab;
		retval.stage="list"

 	return json.encode(retval)
end

function checkIfIsOnline(data)
	local screenhandle = io.popen("screen -ls")
	local value=mysplit(screenhandle:read("*a"),"\n")
	screenhandle:close()
	for k,v in pairs(value) do
		if(string.match(v, streamingScreen))then
			return true
		end
	end

	return false
end


function startStreaming(data)

	if checkIfIsOnline(data) then
		local retval={}
			retval.message="stop streaming first";
			retval.stage="start"

		return json.encode(retval)
	end
	local command=""

	if debug and string.match(data.dev,"test.mp4") then
		command="ffmpeg -re -i "..data.dev.." -f flv "..data.rtmpurl
	else
		command ="ffmpeg -f video4linux2 -re -video_size  "..data.size.." -i "..data.dev.." -f flv "..data.rtmpurl
	end
	startScreenCommand(streamingScreen,command)
	file=io.open (streamingDataFile,"w")
	file:write(data.dev.."\n"..data.rtmpurl)
	file:close()

	local retval={}
		retval.message="OK";
		retval.stage="start"

	return json.encode(retval)
end

function stopStreaming(data)
	stopScreen(streamingScreen)
	local retval={}
		retval.message="OK";
		retval.stage="stop"

	return json.encode(retval)
end

function getStreamingStatus(data)

	local retval={}
	retval.online= checkIfIsOnline(nil)
	if(retval.online)then
	file = io.open(streamingDataFile, "r")
	if not (file==nil) then
		retval.device=file:read("*line")
	  retval.url=file:read("*line")
	end
	end
	retval.stage="status"
	return json.encode(retval)
end

function onmessage(command)
	local retval=""
	data= json.decode(command)

	if data.stage=="getDevices" then
		return getDevices()
	elseif data.stage=="startStreaming" then
		return startStreaming(data)

	elseif data.stage=="stopStreaming" then
		return stopStreaming(data)
	elseif data.stage=="checkStatus" then
		return getStreamingStatus(data)
	end


	return retval;
end
