# progettoreti
progetto di reti informaticje 2020

# general

```bash
cmd --help // to show syntax
``` 

# client

```bash
./dev <porta>
```

## signup

```bash
signup username password [port]
```
req => server
```json
{
    "type" : "signup",
    "username" : "username",
    "password" : "password" 
}
```
resp ✔️
```json
{
    "type" : "signup",
    "status" : 200
}
```
resp ❌
```json
{
    "type" : "signup",
    "status" : 404
}
```

events:
- nothing

## in
```bash
in username password [port]
```
req => server
```json
{
    "username" : "username",
    "password" : "password" 
}
```
resp ✔️
```json
{
    "type" : "in",
    "status" : 200
}
```
resp ❌
```json
{
    "type" : "in",
    "status" : 404
}
```

events:
- server : entry modified
- client : call fake_out() to send old logout if server was taken down

## hanging

```bash
hanging
```

req => server

```json
{
    "type" : "hanging"
}
```
resp ✔️
```json
{
    "type" : "hanging",
    "status" : 200,
    "pending" : [
        {
            "username" : "username",
            "number" : 37,
            "most_recent_timestamp" :  "10/12/2020 13:00:21"
        },
        {

        }
    ]
}
```
resp ❌
```json
{
    "type" : "hanging",
    "status" :  404
}
```
events:
- nothing

## show

```bash
show username
```
*maybe check pending messages number*

req => server

```json
{
    "type" : "show",
    "username" : "username"
}
```
resp ✔️
```json
{
    "type" : "show",
    "status" : 200,
    "pending" : [
        "message",
        "message",
        "message",
        "message",
        "message"
    ]
}
```
resp ❌
```json
{
    "type" : "show",
    "status" :  404
}
```

events:
- server : send_read_notification() to username or buffer if he's offline

## chat
```bash
chat username
```
no request

events:
- see pending_messages(username)

## pending_messages

req => server

```json
{
    // see if pending messages where read
    
    "type" : "chat",
    "username" : "username"
}
```
resp ✔️
```json
{    
    "type" : "chat",
    "username" : "username",
    "status" : 200
}
```
resp ❌
```json
{
    "type" : "show",
    "username" : "username",
    "status" :  404
}
```

events:
- 

## share
```bash
share file_name
```
{

}
## out
```bash
out
```

```json
{
    "type" : "out"
}
```
resp ✔️
```json
{
    "type" : "out",
    "status" : 200
}
```
resp ❌
```json
{
    "type" : "out",
    "status" :  404
}
```

events:
- server : entry
- client : buffer entry if the server is down
 
# server
```bash
./serv [port]
```

## help

commands description

## list
list users
```bash
“username*timestamp*porta”
“username*timestamp*porta”
“username*timestamp*porta”
“username*timestamp*porta”
```

## esc

close server

- no more logins
- no more signup
- users need to save out timestamp


## buffer messages

## send_read_notification

## entry