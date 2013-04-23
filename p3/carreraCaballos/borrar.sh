# Borramos semaforos y memoria compartida
for i in `ipcs -s | awk '{print $2}'`; do 
	ipcrm -s $i;
done

for i in `ipcs -m | awk '{print $2}'`; do 
	ipcrm -m $i;
done

for i in `ipcs -q | awk '{print $2}'`; do 
	ipcrm -q $i;
done

# Matamos los procesos
for i in `ps aux | grep "ejFinal" | awk '{printf "%d ",$2}'`; do
	echo $i;
	`kill -9 $i`;
done

