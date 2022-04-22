# Script Runner test script
cmd("CHOMP_HULL EXAMPLE")
wait_check("CHOMP_HULL STATUS BOOL == 'FALSE'", 5)
