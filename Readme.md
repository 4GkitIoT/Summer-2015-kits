## Project
You can find project details here:
https://4gkit.orange.com


## Basic descriptions
Client module for maintaining communication with server.
Client provide services implemented in lua scripts.
For now following modules are supported:

1.commadline.lua is bash console

2.filetransfer.lua get/put files

3.status.lua status of device (cpu, 4g signal etc.)

4.update.lua save upgrading other scripts

5.modemReconnect.lua watchdog for modem


## Install a development environment
Best way to compile Core is to use docker and docker composer

```
docker-compose up
```

## License
Apache v2
