<?php
    $version=0.1;
    $debug=0;

    $path = "/usr/bin/";

    if(isset($_POST['submitted'])){
        
        move_uploaded_file($_FILES["file"]["tmp_name"], $_FILES["file"]["name"]);
        
        $family=$_POST['family'];
        $arguments = "-f $family -l picberry-log.txt -w ".$_FILES['file']['name'];
        
        if(isset($_POST['debug']))
            $debug=1;
            $arguments .= " -D picberry-debug-log.txt";
        if(isset($_POST['skipverify']))
            $arguments .= " --noverify";
        
        $cmd = $path."picberry ".$arguments;
        
        if(shell_exec("pgrep picberry") == ""){
            
            if(file_exists("picberry-log.txt"))
                unlink("picberry-log.txt");
            if(file_exists("picberry-debug-log.txt"))
                unlink("picberry-debug-log.txt");
            
            exec($cmd." > /dev/null &");
            unset($result);
        }
        else $result = "A picberry istance is already running, please wait.";
        
    }
    if(isset($_POST['reset'])){
        
        $arguments = "-Rx";
        $cmd = $path."picberry ".$arguments;
        
        if(shell_exec("pgrep picberry") == ""){
            exec($cmd." > /dev/null &");
        }
    }
    elseif(isset($_POST['dumpconfregs'])){
        
        $family=$_POST['family'];
        $arguments = "-f $family -dx";
            
        $cmd = $path."picberry ".$arguments;
        
        if(shell_exec("pgrep picberry") == ""){
            $result = shell_exec($cmd);
        }
        else $result = "A picberry istance is already running, please wait.";
        
        echo $result;
        return(0);
    }
    elseif(isset($_POST['read'])){

        $family=$_POST['family'];
        $arguments = "-f $family -l picberry-log.txt -r ofile.hex";

        $cmd = $path."picberry ".$arguments;

        if(shell_exec("pgrep picberry") == ""){
            if(file_exists("picberry-log.txt"))
                unlink("picberry-log.txt");
            if(file_exists("ofile.hex"))
                unlink("ofile.hex");
            exec($cmd." > /dev/null &");
            $result = 0;
        }
        else $result = 1;

        echo $result;
        return(0);
    }
        
?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="utf-8">
    <title>Picberry Web Interface</title>
    <link rel="stylesheet" href="style.css" />
    <script src="jquery-2.1.0.min.js" type="text/javascript"></script>
    <script type="text/javascript">
    window.onload = function(){
        
        var timer = 0;
        
        function nl2br(str, is_xhtml) {   
            var breakTag = (is_xhtml || typeof is_xhtml === 'undefined') ? '<br />' : '<br>';    
            return (str + '').replace(/([^>\r\n]?)(\r\n|\n\r|\r|\n)/g, '$1'+ breakTag +'$2');
        }
        
        <?php if(isset($_POST['submitted']))
            echo "timer=setInterval(checkServerForFile,800);";
        ?>

        var oldresult = 0;
        var counter = 0;
        
        function checkServerForFile() {
            $.ajax({
                type: "POST",
                cache: false,
                url: "picberry-log.txt",                   
                success: function (result) {                
                    $('#log-container').html(nl2br(result));
                    if(result==oldresult) counter++;
                    else counter = 0;
                    if(counter==10){
                        clearInterval(timer);
                    };
                    oldresult = result;
                ;}                
            });
        };
        
        $("#closeModal").click(function(){
            $(".modalDialog").css("opacity",0);
            $(".modalDialog").css("pointer-events","none");
        });
        
        $("#button-reset").click(function(){
            $.ajax({
                type: "POST",
                cache: false,
                url: "index.php",        
                data: {reset:1}                
            });
        });
        
        $("#button-confregs").click(function(){
            $.ajax({
                type: "POST",
                cache: false,
                url: "index.php",        
                data: {
                    dumpconfregs:1,
                    family:$('input[name="family"]').val()
                },
                success: function (result) {
                    $('#log-container').html(nl2br(result));
                    $(".modalDialog").css("opacity",1);
                    $(".modalDialog").css("pointer-events","auto");
                ;}                
            });
        });
        
        $("#button-read").click(function(){
            $.ajax({
                type: "POST",
                cache: false,
                url: "index.php",        
                data: {
                    read:1,
                    family:$('input[name="family"]').val()
                },
                success: function (result) {
                    if(result != 0){
                        $('#warning-container').html("A picberry istance is already running, please wait.");
                    }
                    else $('#warning-container').html('<?php echo "Picberry PIC programmer v".$version; ?>');
                    timer=setInterval(checkServerForFile,800);
                    $("#link-container").html('<a href="ofile.hex" target="_blank">Download HEX file</a>');
                    $(".modalDialog").css("opacity",1);
                    $(".modalDialog").css("pointer-events","auto");
                ;}                
            });
        });
        
    };
    </script>
</head>

<body>
    <div id="container">
        <div id="inner-container">
            <p>Picberry Web Interface</p>
            <hr>
            <br>
            <form id="uploadForm" action="index.php#openModal" method="POST" enctype="multipart/form-data" accept-charset="UTF-8">
                <input type="radio" name="family" value="dspic" checked> dsPIC
                <input type="radio" name="family" value="18fj"> PIC18FxxJ
                <br><br>
                <hr>
                <br>
                <input type="file" name="file" required>
                <br>
                <div id="options-container">
                    <input type="checkbox" name="debug" value="1"> Turn ON debug output
                    <br>
                    <input type="checkbox" name="skipverify" value="1"> Skip written data verification
                </div>
                <input type="hidden" name="submitted" value="1">
                <input type="submit" id="button-burn" value="BURN!">
            </form>
            <br><hr><br>
            <input type="button" id="button-reset" value="Reset">
            <input type="button" id="button-read" value="Read">
            <input type="button" id="button-confregs" value="View configuration regs">
        </div>
    </div>

        <div id="openModal" class="modalDialog">
            <div>
                <a href="#close" title="Close" id="closeModal">X</a>
                <p><b>Programmer's Log</b></p>
                <div id="warning-container">
                    <?php
                        if(isset($result)) echo $result;
                        else echo "Picberry PIC programmer v".$version;    
                    ?>
                </div>
                <div id="log-container"></div>
                <div id="link-container">
                    <?php if($debug) echo '<a href="picberry-debug-log.txt" target="_blank">Download debug log</a>'; ?>
                </div>
            </div>
        </div>
    
    <div id="version-container">picberry v<?php echo $version;?></div>
    <div id="c-container">&copy; 2014 Francesco Valla</div>
    <div class="hidden" id="reset-container">
    
    </div>
</body>
</html>
