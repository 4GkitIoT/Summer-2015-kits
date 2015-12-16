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
    if f then
      local content = f:read("*all")
      f:close()
      return content
    else
      return nil
    end
end

function listofScripts()
  local data=readAll("./scripts.json")
  if(data) then
    print(data)
    data= json.decode(data)
    print(data);
    if(data) then
      return data;
    end
  end
  return {}
end

function writelistofScripts(data)
  file = io.open("./scripts.json", "w")
  data= json.encode(data)
  if file then
    file:write(data)
  end
end


function file_exists(name)
   local f=io.open(name,"r")
   if f~=nil then io.close(f) return true else return false end
end

function reportProgres(data)
  TestFunc("update", data)
end

function calculatemd5(path)
  local md5handle = assert(io.popen("md5sum "..path))
  if md5handle then
    return mysplit(md5handle:read("*a")," ")[1]
  end
  return "0"
end


function listOfModules(scripts)
  local localscripts=listofScripts();
  local rettable={};
  for i,v in ipairs(scripts.files) do
    if(v.type=="bin")then
      rettable[i]={};
      rettable[i].name=v.name;
      rettable[i].exits=file_exists(v.localpath);
      if(rettable[i].exits)then
        rettable[i].md5=calculatemd5(v.localpath);
        rettable[i].size=lfs.attributes(v.localpath, "size")
      end
    elseif (v.type=="script")then
      for j,w in ipairs(localscripts) do
        if(w.name==v.name)then
          rettable[i]={};
          rettable[i].exits=true;
          rettable[i].name=v.name;
          rettable[i].md5="";
        end
      end

    end
  end
  return rettable;
end


function updateScript(name,url)
  io.popen("wget "..url.." -O ".."/tmp/script.sh")
  reportProgres("script fetched "..name)
  io.popen("chmod +x /tmp/script.sh")
  reportProgres("script chmod "..name)
  io.popen("/tmp/script.sh")
  reportProgres("script finished "..name)
  local scripts=listofScripts();
  local thisscript={};
  thisscript.name=name;
  thisscript.md5="";
  table.insert(scripts,thisscript);
  writelistofScripts(scripts);
end

function updateBin(name,path,url)
  local simpleBash=[[
  #!/bin/bash
  cp ]]..path..[[ ]]..path..[[.old;
  if ! wget ]]..url..[[ -O ]]..path..[[;then cp ]]..path..[[.old ]]..path..[[ ;fi
  
  ]]
  io.popen(simpleBash);
end


function onmessage(command)
	local retdata={}
 	data= json.decode(command)
  retdata.stage=data.stage;
  if data.stage=="status" then
    retdata.data=listOfModules(data.scripts)
    return json.encode(retdata);
  elseif data.stage=="update" then
      for k,v in pairs(data.list) do
        if v.kind=="script"then
          updateScript(v.name,v.url);
        end
        if v.kind=="bin"then
          updateBin(v.name,v.path,v.url);
        end

      end
    retdata.stage="done"
    return json.encode(retdata);
  end
end
