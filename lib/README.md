# progettoreti

progetto di reti informatiche 2020

# general

```bash
cmd --help // to show syntax
``` 

## legend
- 🍐 means sender peer (related to the action)
- 🍊 means receiver peer (related to the action)
- 🌳 means server

# 🍐

```bash
./dev <porta>
```

## signup

```bash
signup username password [port]
```

`req => 🌳`

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

resp ❌ (username already used)
```json
{
    "type" : "signup",
    "status" : 404
}
```

## in

```bash
in username password [port]
```

*call this every time 🌳 is down and you need it*

`req => 🌳` if (🌳 was taken down && you logged out) add old_logout_timestamp

```json
{
    "username" : "username",
    "password" : "password",
    "old_logout_timestamp" : "NULL | 10/12/2020 13:00:21:000"
}
```

resp ✔️
```json
{
    "type" : "in",
    "status" : 200
}
```
⚡
- 🌳 : save_entry
- 🍐 : trash old_logout

resp ❌ (wrong username or password)
```json
{
    "type" : "in",
    "status" : 404
}
```

## hanging

```bash
hanging
```

`req => 🌳`

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

## show

```bash
show username
```

`req => 🌳`

```json
{
    "type" : "show",
    "sender" : "username"
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
⚡
- 🌳 : notify(🍐, 🍊)

resp ❌ (username does not exist)
```json
{
    "type" : "show",
    "status" :  404
}
```

## chat

```bash
chat username
```

- `req => 🌳` (to find out if there are buffered messages)

```json
{
    "type" : "chat",
    "username" : "username"
}
```

resp ✔️
```json
{
    "type" : "notify",
    "receiver" : "username",
    "most_recent_timestamp" : "10/12/2020 13:00:21:000"
}
```

⚡
- 🍐 : refresh chat file
- 🍐 : load chat
- 🌳 : forward to 🍊 chat started with 🍐 ???

resp ❌ (username does not exist)
```json
{
    "type" : "chat",
    "status" :  404
}
```


## chat commands

### send_message 'message' + <kbd>Enter</kbd>

`req => 🍊` if up || `req => 🍊s` if group

```json
{
    "type" : "send_message",
    "content" : "content",
    "timestamp" : "10/12/2020 13:00:21:000"
}
```
⚡
- 🍊 : receive and display

`req => 🌳` if not up && if not group

```json
{
    "type" : "send_message",
    "content" : "content",
    "timestamp" : "10/12/2020 13:00:21:000",
    "receiver" : "username"
}
```

⚡
- 🌳 : buffer_message

### quit '\q' + <kbd>Enter</kbd>

- close connection to 🍊s

⚡
- other 🍊 need to close connection with 🍐
- if group is composed by 2 maybe call chat ???

### ls_user '\u' + <kbd>Enter</kbd>

`req => 🌳`

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

### add_user '\a username’ + <kbd>Enter</kbd>

`req => 🌳`

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
⚡ (waiting cause you need to know 🍊 port)
- 🍐 : new_user_is_added(username, port) => other 🍊s
- 🍐 : send_users_in_chat_to_new_user(username) => new 🍊 

resp ❌ (username does not exists or 🍊 is on another group)
```json
{
    "type" : "add_user",
    "status" :  404
}
```

## send_users_in_chat_to_new_user

`req => new 🍊`

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
⚡
- new 🍊 adds starts connection to other group 🍊s

## new_user_is_added

`req => all other 🍊s`

```json
{
    "type" : "new_user_is_added",
    "username" : "username",
    "port" : 4192
}
```

- other group 🍊s starts connection to the new 🍊 

## share

```bash
share file_name
```

`req => 🍊s` (base-64-encoding)

```json
{
    "type" : "share",
    "content" : "encoded"
}
```

## out

```bash
out
```

close server connection

⚡
- 🍐 : buffer entry if the 🌳 is down
- 🌳 : save_entry
 
# 🌳

```bash
./serv [port]
```

## help

commands description

## list

list users

```bash
username * timestamp * porta
username * timestamp * porta
username * timestamp * porta
username * timestamp * porta
```

## esc

close 🌳

- no more logins
- no more signup
- users need to save log out timestamp

⚡
- all 🍊s save that 🌳 is offline

## buffer_message

save

```json
{   
    "sender" : "username",
    "receiver" : "username",
    "content" : "content",
    "timestamp" : "10/12/2020 13:00:21:000",
}
```

## notify


`🍐 is online => notify that 🍊 has read to 🍐`

```json
{
    "type" : "notify",
    "receiver" : "username",
    "most_recent_timestamp" : "10/12/2020 13:00:21:000"
}
```
⚡
- 🍐 : refresh chat file with **

`🍐 is offline => buffer` 

```json
{
    "sender" : "username",
    "receiver" : "username",
    "most_recent_timestamp" : "10/12/2020 13:00:21:000"
}
```

## save_entry

save

```json
{
    "username" : "username",
    "type" : "in | out",
    "timestamp" : "10/12/2020 13:00:21:000",
    "port" : 2349
} 
```