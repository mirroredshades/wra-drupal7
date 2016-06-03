<?PHP
/*
 _____                    ______ _   _ ______
|  __ \                   | ___ \ | | || ___ \
| |  \/_ __ ___  ___ _ __ | |_/ / |_| || |_/ /
| | __| '__/ _ \/ _ \ '_ \|  __/|  _  ||  __/
| |_\ \ | |  __/  __/ | | | |   | | | || |
 \____/_|  \___|\___|_| |_\_|   \_| |_/\_|
----------------------------------------------
(C)BOBAK 2008                             V2.0
----------------------------------------------
web   : http://www.greenphp.com
email : bobak@greenphp.com
----------------------------------------------
*/

/**
 * Management of FDF (PDF Forms : Make,Load,Save, convert tu PDF)
 * Workin with pdftk (http://www.pdfhacks.com/pdftk)
 *
 * @author  Bobak
 * @package pdf
 * @subpackage fdf
 */

class pdf_fdf
{
/**
 * make a file FDF
 *
 * @param string $FichierIn Name PDF file reference
 * @param array $Data1 List of variables to include
 * @param array $Data2 List of variables to include unprotected
 * @return FDF
 */
public static function Make($FichierIn,$Data1,$Data2="")
{
$Out = "%FDF-1.2\n%????\n";
$Out .= "1 0 obj \n<< /FDF << /Fields [\n";

if ($Data1)
  {
  foreach ($Data1 as $DataKey=>$DataVal)
         {
         $DataVal=str_replace("\r\n","\n",$DataVal);
         $DataVal=str_replace("\r"  ,"\n",$DataVal);
         $Out.= "<< /T (".addcslashes($DataKey,"\n\r\t\\()").") /V (".addcslashes($DataVal,"\n\r\t\\()").") >> \n";
         }
  }

if (is_array($Data2))
  {
  foreach ($Data2 as $DataKey=>$DataVal)
         {
         $DataVal=str_replace("\r\n","\n",$DataVal);
         $DataVal=str_replace("\r"  ,"\n",$DataVal);
         $Out.="<< /T (".addcslashes($DataKey,"\n\r\t\\()").") /V /".$DataVal." >> \n";
         }
  }

$Out.= "]\n/F ($FichierIn) >>";

$Out.= ">>\nendobj\ntrailer\n<<\n";
$Out.= "/Root 1 0 R \n\n>>\n";
$Out.= "%%EOF";

return $Out;
}

/**
 * Save a file FDF
 *
 * @param string $FichierIn Name PDF file reference
 * @param array $Data Fields
 * @param string $FichierOut Name File final
 * @return bool
 */
public static function Save($FichierIn,$Data,$FichierOut)
{
if ($FId=fopenw($FichierOut,'w'))
  {
  $Data=self::Make($FichierIn,$Data);
  fwrite($FId,$Data,strlen($Data));
  $Out=TRUE;
  }
  else
  {$Out=FALSE;}
return $Out;
}

/**
 * Load a file FDF
 *
 * @param string $Fichier File to analyse
 * @param bool $PDF Type of file (PDF | FDF)
 * @return string
 */
public static function Load($Fichier,$PDF=FALSE)
{
$Out=array();

if ((file_exists($Fichier))and($Fichier>''))
  {
  $Data=File($Fichier);

  if ($PDF)
    {
    $Nb=count($Data);
    for ($i=4;$i<$Nb;$i++)
       {
       $DataOut=array();
       if(eregi("/F \(([^)]*)\)",$Data[$i],$DataOut))
         {
         $Out=$DataOut[1];
         break;
         }
       }
    }
    else
    {
    $Nb=count($Data)-9;
    for ($i=4;$i<$Nb;$i++)
       {
       $DataOut=array();

       $In=$Data[$i];
       $In=str_replace('\)','§PF§',$In);
       $In=str_replace('\r\n',EOL,$In);
       $In=str_replace('\n',EOL,$In);
       $In=str_replace('\r',EOL,$In);
       $In=stripslashes($In);

       eregi("/T \(([^)]*)\) /V \(([^)]*)\)",$In,$DataOut);
       $Out[$DataOut[1]]=str_replace("§PF§",")",$DataOut[2]);
       }
     }
  }

return $Out;
}

/**
 * Transform an FDF PDF
 *
 * @param string $FileFDF Filename a PDF transfrmer
 * @param string $FilePDF_Out Filename of output
 * @return string
 */
public static function FDF2PDF($FileFDF,$FilePDF_Out='')
{
//Laod FDF
$FilePDF_In=self::Load($FileFDF,TRUE);

//Prepare the name of the PDF
$FilePDF_Ref=basename($FilePDF_In);

//Find the name of the output file
if (!$FilePDF_Out)
  {$FilePDF_Out=str_replace(".fdf",".pdf",$FileFDF);}

//Retrieve the PDF
if ($FilePDF_Ref AND !file_exists($FilePDF_Ref))
  {copy($FilePDF_In,$FilePDF_Ref);}

//Empty the old version of PDF if it exists
if ($FilePDF_Out AND file_exists($FilePDF_Out))
  {unlink($FilePDF_Out);}

//Converted
exec("pdftk $FilePDF fill_form $FileFDF output $FilePDF_Out");

return $FilePDF_Out;
}

/**
 * HTML header for FDF
 *
 */
public static function Header()
{
header('Content-type: application/vnd.fdf');
//header( "Content-type: application/vnd.adobe.xfdf");
}
}
?>