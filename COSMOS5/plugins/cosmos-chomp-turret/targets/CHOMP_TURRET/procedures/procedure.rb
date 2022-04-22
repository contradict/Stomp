# Script Runner test script
cmd("CHOMP_TURRET EXAMPLE")
wait_check("CHOMP_TURRET STATUS BOOL == 'FALSE'", 5)
