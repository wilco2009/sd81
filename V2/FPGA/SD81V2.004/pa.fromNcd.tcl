
# PlanAhead Launch Script for Post PAR Floorplanning, created by Project Navigator

create_project -name SD81V2.004 -dir "/home/ise/ISEWebpack/SD81V2.004/planAhead_run_3" -part xc6slx9tqg144-3
set srcset [get_property srcset [current_run -impl]]
set_property design_mode GateLvl $srcset
set_property edif_top_file "/home/ise/ISEWebpack/SD81V2.004/SD81.ngc" [ get_property srcset [ current_run ] ]
add_files -norecurse { {/home/ise/ISEWebpack/SD81V2.004} {ipcore_dir} }
set_property target_constrs_file "SD81XC6.ucf" [current_fileset -constrset]
add_files [list {SD81XC6.ucf}] -fileset [get_property constrset [current_run]]
link_design
read_xdl -file "/home/ise/ISEWebpack/SD81V2.004/SD81.ncd"
if {[catch {read_twx -name results_1 -file "/home/ise/ISEWebpack/SD81V2.004/SD81.twx"} eInfo]} {
   puts "WARNING: there was a problem importing \"/home/ise/ISEWebpack/SD81V2.004/SD81.twx\": $eInfo"
}
