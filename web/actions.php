<?php
    $path = "/usr/bin/";

    if(isset($_POST['submitted'])){
        
        $family=$_POST['family'];
        $arguments = "-x -f $family -l /tmp/picberry-log.txt -w /tmp/picberry-write.hex";
        
        $debug = "";
        
        if(isset($_POST['debug'])){
            $debug = "&d=1";
            $arguments .= " -D";
        }
        if(isset($_POST['skipverify']))
            $arguments .= " -n";
        
        $cmd = $path."picberry ".$arguments." &";
        
        if(shell_exec("pgrep picberry") == ""){
            
             if(file_exists("/tmp/picberry-write.hex"))
                unlink("/tmp/picberry-write.hex");
            move_uploaded_file($_FILES["file"]["tmp_name"], "/tmp/picberry-write.hex");
            
            if(file_exists("picberry-log.txt"))
                unlink("picberry-log.txt");
            
            $descriptorspec = array(
                2 => array("file", "/tmp/picberry-state.txt", "w")
            );

            $process = proc_open($cmd, $descriptorspec, $pipes);
            header("location: index.php?s=0".$debug."#openModal");
        }
        else header("location: index.php?s=1".$debug."#openModal");   
    }
    elseif(isset($_POST['reset'])){
        
        $arguments = "-Rx";
        $cmd = $path."picberry ".$arguments;
        
        if(shell_exec("pgrep picberry") == ""){
            exec($cmd." > /dev/null &");
        }
    }
    elseif(isset($_POST['query'])){
        do{
            $raw_text = file_get_contents("/tmp/picberry-state.txt");
            $raw_parts = preg_split("/\n/", $raw_text, -1, PREG_SPLIT_NO_EMPTY);
        }
        while(end($raw_parts) == $_POST['state']);
        
        if($_POST['state'] == -1){
            echo $raw_parts[0];
            return 0;
        }
        elseif(end($raw_parts) == "END"){
            if(file_exists("/tmp/picberry-state.txt"))
                unlink("/tmp/picberry-state.txt");
            if(file_exists("/tmp/picberry-write.hex"))
                unlink("/tmp/picberry-write.hex");
            if(file_exists("/tmp/picberry-log.txt")){
                $res = exec("cp /tmp/picberry-log.txt .");
                unlink("/tmp/picberry-log.txt");
            }
            echo $raw_parts[count($raw_parts)-2];
        }
        else echo end($raw_parts);
    }; 

?>