# progettoreti
progetto di reti informatiche 2020

# general

```bash
cmd --help // to show syntax
``` 

# 🍐

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
    "status" : 404  // username already used
}
```

events:
- nothing

## in
```bash
in username password [port]
```
call this every time server is down and you need it

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
    "status" : 404  // wrong username or password
}
```

events:
- server : entry modified
- 🍐 : call fake_out() to send old logout if (server was taken down && you logged out)

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
            "most_recent_timestamp" :  "10/12/2020 13:00:21:000"
        },
        {

        }
    ]
}
```

events:
- nothing

## show

```bash
show username
```
*maybe hanging => show if pending from user > 1*

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
    "status" :  404 // username does not exist
}
```

events:
- server : notify()

## chat
```bash
chat username
```

- req => server to find out if there are buffered messages

```json
{
    "type" : "chat",
    "username" : "username"
}

resp ✔️
```json
{
    "type" : "notify",
    "receiver" : "username",
    "most_recent_timestamp" : "10/12/2020 13:00:21:000"
}
```
events:
- client : refresh chat file
- client : load chat

resp ❌
```json
{
    "type" : "chat",
    "status" :  404 // username does not exist
}
```


## chat commands

### send_message 'message' + <kbd>Enter</kbd>

req => 🍐 if up
```json
{
    "type" : "send_message",
    "content" : "content",
    "timestamp" : "10/12/2020 13:00:21:000"
}
```
req => server
```json
{
    "type" : "send_message",
    "content" : "content",
    "timestamp" : "10/12/2020 13:00:21:000",
    "receiver" : "username"
}
```

events:
- server : buffer_message()

*** check for group messages

### quit '\q' + <kbd>Enter</kbd>

- close connection to 🍐s

events:
- other 🍐 need to close connection with you
- if group is composed by 2 maybe call chat

### ls_user '\u' + <kbd>Enter</kbd>

req => server

```json
{
    "type" : "ls"
}
```
resp ✔️
```json
{
    "type" : "ls",
    "status" : 200,
    "usernames" : [
        "username",
        "username",
        "username"
    ]
}
```
resp ❌
```json
{
    "type" : "ls",
    "status" :  404
}
```

### add_user '\a username’ + <kbd>Enter</kbd>

req => server

```json
{
    "type" : "add_user",
    "username" : "username"
}
```
resp ✔️
```json
{
    "type" : "add_user",
    "status" : 200,
    "port" : 8080
}
```
resp ❌
```json
{
    "type" : "add_user",
    "status" :  404  // server error o 🍐 is on another group
}
```
events:
- 🍐 : new_user_is_added(username, port) => other 🍐s
- 🍐 : send_users_in_chat_to_new_user(username) => new 🍐 

## send_users_in_chat_to_new_user

req => new 🍐

```json
{
    "type" : "send_users_in_chat",
    "users" : [
        {
            "username" : "username",
            "port" : 3289 
        },
        {

        }
    ]
}
```

## new_user_is_added

req => all other 🍐s

```json
{
    "type" : "new_user_is_added",
    "username" : "username",
    "port" : 4192
}
```

## share
```bash
share file_name
```
```json
{
    "type" : "share",
    "content" : "encoded" //base-64-encoding
}
```
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
- server : entry()
- 🍐 : buffer entry if the server is down
 
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


## buffer_message

## notify

🍐 : 
- online => notify read to 🍐
- offline => buffer 
```json
{
    "type" : "notify",
    "receiver" : "username",
    "most_recent_timestamp" : "10/12/2020 13:00:21:000"
}
```

## entry