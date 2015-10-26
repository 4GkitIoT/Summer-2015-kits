--
-- Copyright (C) 2015 Orange
--
-- This software is distributed under the terms and conditions of the 'Apache-2.0'
-- license which can be found in the file 'LICENSE' in this package distribution
-- or at 'http://www.apache.org/licenses/LICENSE-2.0'.
--

require ("json")

local devicePath="/dev/ttyACM0"
local pathToConfig="/root/4gkit/Core/config.json"
function file_exists(name)
   local f=io.open(name,"r")
   if f~=nil then io.close(f) return true else return false end
end

function mysplit(inputstr, sep)
  -- split function 
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

function sendSerialCommand(command)
local wserial=assert(io.open(devicePath,"w"))
if wserial then
    wserial:write(command.."\n")
    wserial:flush()
    wserial:close()
end
end


function readSerialState(expectedNumber)
  local rserial=assert(io.open(devicePath,"r"))
  local retry
  if rserial then
    local data=""

    for i=1,expectedNumber do
      data=data..rserial:read()
    end
    return data
  end
end

function delay(time)
  os.execute("sleep "..time)
end

function getMac()
  local macHandle=io.popen("ifconfig")
  local ifconfig=macHandle:read("*a")
  macHandle:close()
  local netrows=mysplit(ifconfig,"\n")
      for k,v in pairs(netrows)do
        local interfacedata=mysplit(v," ")
          if(string.match(interfacedata[1],"eth"))then
            local ipdata=mysplit(netrows[k+1]," ")
            if(string.match(ipdata[2],"addr:192.168.15."))then
              return interfacedata[5]
            end
          end
      end
    return "00:00:00:00:00:00"
end


function saveData(imei,mac)
  --saving data in temp file for device status etc
  local _imeiMacPath="/tmp/4gimeimac"
  local _macImei = io.open(_imeiMacPath, "w")
  local _dat_={}
  _dat_.imei=imei
  _dat_.mac=mac
  local _datatosave=json.encode(_dat_)
  if _macImei then
    _macImei:write(_datatosave)
    _macImei:close()
  end
end

function writeDownBoardId(IMEI)
  --write device id in config file
    file = io.open(pathToConfig, "r")
    if file then
        local configjson=file:read("*all")
        file:close()
        local macaddres=getMac()
        saveData(IMEI,macaddres)
        macaddres=string.gsub(macaddres,":","")
        macaddres=string.upper(macaddres)



        local boardId=IMEI:sub( 10, 15 )..macaddres:sub( 7, 12 )

        if configjson then
                  local config=json.decode(configjson)
                  if (config.boardId~=boardId)then
                      config.boardId=boardId


                        configjson=json.encode(config)

                        file = io.open(pathToConfig, "w")
                        if file then
                          file:write(configjson)
                          restartCore()
                        end
                  end
        end
    end
end



function checkImei()
    sendSerialCommand("AT+CGSN")
    local data=string.match(readSerialState(3),"%d+")
    if tonumber(data) then
        if(string.len(data)==15) then
            print("IMEI")
            print(data)

            writeDownBoardId(data)
        else
            delay(1)
            checkImei()
        end
    else
      delay(1)
      checkImei()
    end
end


function addscanbands()
  print(">>>>>>>>>>>MODEM INIT SEQUENCE"..'AT!="addscanband 3"')
  sendSerialCommand('AT!="addscanband 3"')
  delay(4)
    print(">>>>>>>>>>>MODEM INIT SEQUENCE"..'AT!="addscanband 7"')
  sendSerialCommand('AT!="addscanband 7"')
  delay(4)
    print(">>>>>>>>>>>MODEM INIT SEQUENCE"..'AT!="addscanband 20"')
  sendSerialCommand('AT!="addscanband 20"')
  delay(4)
end


function powerUpModem()
  sendSerialCommand('AT+CFUN=1')
  delay(3)
end

function restartCore()
  os.execute("/etc/init.d/4gstartercore restart")
end

function setTTY()
  os.execute("stty -F /dev/ttyACM0 57600 -echo -echoe -icanon -opost")
end


while not file_exists(devicePath) do
  delay(2)
end
delay(8)
print(">>>>>>>>>>>MODEM INIT SEQUENCE")
setTTY()
addscanbands()
powerUpModem()
checkImei()
