--
-- Copyright (C) 2015 Orange
--
-- This software is distributed under the terms and conditions of the 'Apache-2.0'
-- license which can be found in the file 'LICENSE' in this package distribution
-- or at 'http://www.apache.org/licenses/LICENSE-2.0'.
--

lpty = require "lpty"

if bashsessions==nil then
	bashsessions={}
end

if bashtimeouts==nil then
	bashtimeouts={}
end

function initNewSession(session)
	bashsessions[session]=lpty.new()
	if not bashsessions[session]:hasproc() then
		bashsessions[session]:startproc("bash")
	end
end

function clearInactiveSessions()

	local timeout=600
	local now=os.time()

	for id,time in pairs(bashtimeouts) do
			if ((now-time)>timeout) then
				bashsessions[id]=nil
				bashtimeouts[id]=nil
			end
	end
end

function getBashSession(session)
		bashtimeouts[session]=os.time()
		clearInactiveSessions()
		if bashsessions[session]~=nil then
			return bashsessions[session]
		else
			initNewSession(session)
			return bashsessions[session]
		end
end


function getResponseText(bash,session)
	-- local maxpayload=200
	-- local result=""
	-- local data=""

	-- while data do
	-- 	result=result..data
	--     data=bash:read(1)
	-- 	if(not data) or string.len(result)>maxpayload then
	-- 		return result
	-- 	end
	--  end
	-- return ""
	local data=bash:read(1)
	if data then
		return data
	else
		return ""
	end
end

function onmessage(command,session)

	if session==nil then
		session="test"
	end
	local bash=getBashSession(session)
	if (command=="")then
		return getResponseText(bash,session)
	else
		bash:send(command)
		return bash:read(1);
	end	
end