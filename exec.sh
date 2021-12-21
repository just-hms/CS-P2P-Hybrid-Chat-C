make

printf "Compilazione eseguita. Premi invio per eseguire..." 
read _

gnome-terminal -x sh -c "./serv 4242; exec bash"

for port in {5001..5003}
do
    gnome-terminal -x sh -c "./dev $port; exec bash"
done