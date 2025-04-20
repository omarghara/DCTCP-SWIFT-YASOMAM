
#this ensures the script starts from where it is
cd "$(dirname "$0")"

#clear the contents of the results file 
#>"results.txt"

#go back to the ns3 directory
cd ..
make

for num_of_flows in 100 250; do
    echo "Running simulation with num_of_flows=$num_of_flows"

    #running the simulation now

    #this is the original comman line that we want to change the num of flows
    ./ns3 run scratch/dctcp_cmd.cc -- --num_of_flows=$num_of_flows  --mode=1 --k=4  --simulation_stop_time=15 --protocol=TcpDctcp

    ./ns3 run scratch/dctcp_cmd.cc -- --num_of_flows=$num_of_flows  --mode=1 --k=4  --simulation_stop_time=15 --protocol=TcpDctcp

    ./ns3 run scratch/dctcp_cmd.cc -- --num_of_flows=$num_of_flows  --mode=1 --k=4  --simulation_stop_time=15 --protocol=TcpDctcp

    echo -e "\n------ End of Simulation for num_of_flows=$num_of_flows ------\n" 

done

#cd scratch
#./analyse_results.py 
#cd ..
echo "All simulations completed. Results saved in scratch/results.txt and in results.xlsx"

#------------------------------------------------------------------------------------------------------------