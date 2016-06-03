<?php

function mail_attachment ($from , $to, $subject, $message, $attachments){
$email_from = $from; // Who the email is from
$email_subject = $subject; // The Subject of the email
$email_txt = $message; // Message that the email has in it

$email_to = $to; // Who the email is to
$headers = "From: ".$email_from;

$semi_rand = md5(time());
$mime_boundary = "==Multipart_Boundary_x{$semi_rand}x";

$headers .= "\nMIME-Version: 1.0\n" .
"Content-Type: multipart/mixed;\n" .
" boundary=\"{$mime_boundary}\"";

$email_message .= "This is a multi-part message in MIME format.\n\n" .
"--{$mime_boundary}\n" .
"Content-Type:text/html; charset=\"iso-8859-1\"\n" .
"Content-Transfer-Encoding: 7bit\n\n" . $email_txt . "\n\n";

foreach ($attachments as $a) {
$data = chunk_split(base64_encode($a['data']));

$email_message .= "--{$mime_boundary}\n" .
"Content-Type: {$a['mime']};\n" .
" name=\"{$a['filename']}\"\n" .
"Content-Transfer-Encoding: base64\n\n" .
$data . "\n\n";
}

$email_message .= "--{$mime_boundary}--\n";

$ok = @mail($email_to, $email_subject, $email_message, $headers);

if($ok) {
} else {
die("There was a problem sending the email. Please check your inputs.");
}
}

function mail_attachment_bcc ($from , $to, $bcc, $subject, $message, $attachments){
$email_from = $from; // Who the email is from
$email_subject = $subject; // The Subject of the email
$email_txt = $message; // Message that the email has in it

$email_to = $to; // Who the email is to
$headers = "From: ".$email_from;
$headers .= "\nBcc: ".$bcc;

$semi_rand = md5(time());
$mime_boundary = "==Multipart_Boundary_x{$semi_rand}x";

$headers .= "\nMIME-Version: 1.0\n" .
"Content-Type: multipart/mixed;\n" .
" boundary=\"{$mime_boundary}\"";

$email_message .= "This is a multi-part message in MIME format.\n\n" .
"--{$mime_boundary}\n" .
"Content-Type:text/html; charset=\"iso-8859-1\"\n" .
"Content-Transfer-Encoding: 7bit\n\n" . $email_txt . "\n\n";

foreach ($attachments as $a) {
$data = chunk_split(base64_encode($a['data']));

$email_message .= "--{$mime_boundary}\n" .
"Content-Type: {$a['mime']};\n" .
" name=\"{$a['filename']}\"\n" .
"Content-Transfer-Encoding: base64\n\n" .
$data . "\n\n";
}

$email_message .= "--{$mime_boundary}--\n";

$ok = @mail($email_to, $email_subject, $email_message, $headers);

if($ok) {
} else {
die("There was a problem sending the email. Please check your inputs.");
}
}
?>