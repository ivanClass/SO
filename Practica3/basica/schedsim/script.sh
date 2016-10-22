#!/bin/bash
##A) SOLICITAMOS NOMBRE FICHERO A SIMULAR Y NUM.MAX CPUS
existeFichero=0
while [ $existeFichero != 1 ]
do
	echo 'Introduce el nombre del fichero a simular: '
	read nombreFichero

	if (test -f examples/$nombreFichero); then
		existeFichero=1
	else
		echo 'El fichero no existe'
	fi
done


echo 'Introduce número máximo de CPUs a utilizar: '
read maxCPUS
while (($maxCPUS<1 || $maxCPUS>8))
do
	echo 'El valor tiene que estar entre 1 y 8.'
	echo 'Introduce número máximo de CPUs a utilizar: '
	read maxCPUS
done

##B) CREACIÓN DE DIRECTORIO RESULTADOS
if test -d resultados; then
	rm -rf resultados
fi
mkdir resultados

##C) PARA CADA UNO DE LOS PLANIFICADORES GUARDAMOS SUS RESULTADOS
for planif in FCFS SJF RR PRIO
do
	for ((cpu=1; cpu<=$maxCPUS; cpu++))
	do
		./schedsim -i examples/$nombreFichero -s $planif -n $cpu
		for ((cpuAux=0; cpuAux<$cpu; cpuAux++))
		do
			mv ./CPU_$cpuAux.log ./resultados/$planif-CPUs$cpu-CPU$cpuAux.log
		done

		cd ../gantt-gplot/
		for ((cpuAux=0; cpuAux<$cpu; cpuAux++))
		do
			./generate_gantt_chart ../schedsim/resultados/$planif-CPUs$cpu-CPU$cpuAux.log
		done
		cd ../schedsim/

	done

done
