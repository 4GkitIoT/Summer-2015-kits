--
-- Copyright (C) 2015 Orange
--
-- This software is distributed under the terms and conditions of the 'Apache-2.0'
-- license which can be found in the file 'LICENSE' in this package distribution
-- or at 'http://www.apache.org/licenses/LICENSE-2.0'.
--
require ("os")
require ("json")

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

function readAll(file)
    local f = io.open(file, "rb")
    local content = f:read("*all")
    f:close()
    return content
end

function getImeiAndMac()
  imeiMacPath="/tmp/4gimeimac"
  imeiMacFile = io.open(imeiMacPath, "r")

  if imeiMacFile then
    local imeimactext=imeiMacFile:read("*all")
    imeiMacFile:close()
    if imeimactext then
      local imeimacdata=json.decode(imeimactext)

      return imeimacdata.imei, imeimacdata.mac
    end
  end
end

function file_exists(name)
   local f=io.open(name,"r")
   if f~=nil then io.close(f) return true else return false end
end

function fourGstatus()
  local devicepath="/dev/ttyACM0"

  if file_exists(devicepath) then

  local percentSignal
  local CEREG
  local wserial=assert(io.open(devicepath,"w"))
  if wserial then
      wserial:write("AT+CSQ\n")
      wserial:write("AT+CEREG?\n")

      wserial:flush()
      local rserial=assert(io.open("/dev/ttyACM0","r"))

      rserial:read()
      rserial:read()
      local signal=rserial:read()
      rserial:read()
      rserial:read()
      rserial:read()
      rserial:read()
      rserial:read()
      rserial:read()
      rserial:read()
      local net=rserial:read()
      wserial:close()
      if net then
        local netsplited=mysplit(net," ")
        CEREG=netsplited[2]

      end


      if signal then
        local signalsplited=mysplit(signal," ")
        if signalsplited[2] then
          local RSSI=mysplit(signalsplited[2],",")
          RSSI=tonumber(RSSI[1])
          local dbm=-113

          if RSSI>1 and RSSI<31 then
            dbm=-109+RSSI*2+2
          elseif RSSI==31 then
            dbm=-51
          elseif RSSI==1 then
              dbm=-100
          end

          percentSignal=2*(dbm+113)
        end
      end
  end

  return percentSignal,CEREG

  end
end


function extractSignal(text)
  local signaltext=mysplit(text,"\n")[3]

  if signaltext then

  return string.gsub(mysplit(signaltext," ")[4],"[.]","")
  else
    return "0"
  end
end

function getUploadDownloadSpeeds(ifacename)

  local netstats=readAll("/proc/net/dev")
  local netrows=mysplit(netstats,"\n")

      for k,v in pairs(netrows)do
        local interfacedata=mysplit(v," ")
          if(string.match(interfacedata[1],ifacename))then

            return tonumber(interfacedata[2]), tonumber(interfacedata[10])
          end
      end
end


function onmessage(command)

  local iswifi=true;

	local retdata={}
 	data= json.decode(command)

    --TODO
   retdata.up_time=string.match(readAll("/proc/uptime"), "([^ ]+)")
		local iphandle = assert(io.popen("wget -qO- http://ipecho.net/plain ; echo"))
   retdata.ip=string.match(iphandle:read("*a"),"([^ ]+)")
		iphandle:close()
    local storagehandle=io.popen("df")
    local storagestring=storagehandle:read("*a")
   retdata.free_mem=mysplit(mysplit(storagestring,"\n")[2]," " )[4]
   retdata.total_mem=mysplit(mysplit(storagestring,"\n")[2]," " )[2]
    storagehandle:close()
		local cpuhandle=io.popen("grep 'cpu ' /proc/stat | awk '{cpu_usage=($2+$4)*100/($2+$4+$5)} END {print cpu_usage}' ")
    local cpu=cpuhandle:read("*a")
   retdata.cpu=math.floor(tonumber(cpu))
	   cpuhandle:close();
   retdata.temp=0 --TODO
   retdata.free_ram	=	string.match(string.match(readAll("/proc/meminfo"), "MemFree:%s* %d+"),"%d+")
   retdata.total_ram=	string.match(readAll("/proc/meminfo"), "%d+")
   if iswifi then
	    retdata.net_type='4G'
      retdata.signal,retdata.network=fourGstatus()
      retdata.download,retdata.upload=getUploadDownloadSpeeds("eth");
      retdata.imei, retdata.mac=getImeiAndMac()

   else
     retdata.net_type='4G'
     retdata.signal=99
   end


   return json.encode(retdata)
end
