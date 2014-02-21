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
        
        function progress(percent, element) {
            var progressBarWidth = percent * $(element).width() / 100;
            $(element).find('div').animate({ width: progressBarWidth }, 500).html(percent + "%&nbsp;");
            if(percent == 100){
                $(element).find('div').css("background-color","white");
                $(element).find('div').css("color","black");
            }
        }
        
        <?php if(isset($_GET['s']))
            echo "checkServerForState(-1);";
        ?>

        var oldresult = 0;
        var counter = 0;
        
        function checkServerForState(state) {
            $.ajax({
                type: "POST",
                cache: false,
                data: {
                    query: 1,
                    state: state
                },
                url: "actions.php",                   
                success: function (result) {
                    var splitted = result.split('@');
                    if(splitted[0] == 'MSG'){
                        $('#warning-container').append(splitted[1]);
                        return 0;
                    }
                    else if((splitted[0] == 'WRT' || splitted[0] == 'VRF') && splitted[1] != 'SKP')
                        progress(splitted[1], '#'+splitted[0]+'-progressBar');
                    else if(splitted[0] == 'PRT')
                        $('#log-container').append(splitted[1]+' detected');
                    if(result == 'VRF@100' || result == 'VRF@SKP'){
                        if(result == 'VRF@SKP')
                            $('#VRF-progressBar').find('div').html('SKIPPED');
                        $('#link-container').append('<hr><b>PIC programmed!</b>');
                        <?php if(isset($_GET['d'])){?>
                        $('#link-container').append('<br><br><a href="picberry-log.txt">View debug log</a>');
                        <?php ;} ?>
                        return 0;
                    }
                    checkServerForState(result);
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
                url: "actions.php",        
                data: {reset:1}                
            });
        });        
    };
    </script>
</head>

<body>
    <div id="container">
        <div id="inner-container">
            <p><b>PICBERRY Web Interface</b></p>
            <form id="uploadForm" action="actions.php" method="POST" enctype="multipart/form-data" accept-charset="UTF-8">
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
        </div>
    </div>

        <div id="openModal" class="modalDialog">
            <div>
                <a href="#close" title="Close" id="closeModal">X</a>
                <p>picberry PIC programmer v0.1</p>
                <hr>
                <div id="warning-container">
                    <?php
                        if(isset($_GET['s']) && $_GET['s'] == 1)
                            echo "<br>A picberry istance is already running, please wait.<br>";
                    ?>
                </div>
                <div id="log-container"></div>
                <div id="progbar-container">
                    <table>
                        <tr>
                            <td width="20%">Write: </td>
                            <td width="80%"><div class="progressBar" id="WRT-progressBar"><div></div></div></td>
                        </tr>
                        <tr>
                            <td>Verify: </td>
                            <td><div class="progressBar" id="VRF-progressBar"><div></div></div></td>
                        </tr>
                    </table>
                </div>
                <div id="link-container"></div>
            </div>
        </div>
    
    <div id="c-container">
        &copy; 2014 Francesco Valla - hosted on <a href="https://github.com/WallaceIT/picberry">Github</a>
    </div>
</body>
</html>
