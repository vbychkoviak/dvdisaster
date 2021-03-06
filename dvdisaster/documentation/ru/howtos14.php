<?php
# dvdisaster: English homepage translation
# Copyright (C) 2004-2010 Carsten Gnörlich
#
# UTF-8 trigger: äöüß 
#
# Include our PHP sub routines, then call begin_page()
# to start the HTML page, insert the header, 
# navigation and news if appropriate.

require("../include/dvdisaster.php");
require("../include/screenshot.php");

begin_page();

howto_headline("Поиск ошибок на носителях", "Дополнительные настройки", "images/scan-icon.png");
?>

<!--- Insert actual page content below --->

<?php begin_screen_shot("Вкладка \"Привод\".","scan-prefs-drive-adv.png"); ?>
<b>Игнорирование неисправимых ошибок.</b>
Обычно dvdisaster прерывает проверку, когда привод сообщает
о таких неисправимых внутренних ошибках, как проблемы с механикой. Это делается
для того, чтобы избежать повреждения привода. Но некоторые приводы ошибочно сообщают
о таких проблемах, когда их сбивают с толку поврежденные носители. Если у вас такой
привод, то вы можете использовать этот параметр для принудительного продолжения проверки.<p>
<b>Извлекать носитель после успешного чтения.</b>
Если этот параметр включен, dvdisaster попытается извлечь носитель после успешной проверки. 
Однако извлечению носителя может препятствовать операционная система, поэтому это не обязательно сработает. Например,
 если после вставления носителей открывается окно для выполнения содержимого, то 
автоматическое извлечение носителя может быть невозможным.
<p>
<?php end_screen_shot(); ?>

<pre> </pre>

<?php begin_screen_shot("Вкладка \"Попытки чтения\".","scan-prefs-read-attempts-adv.png"); ?>
<b>Пропуск секторов после ошибки чтения.</b>
Попытки прочитать поврежденные сектора требуют много времени.
Поскольку после нахождения ошибки чтения есть вероятность встретить еще один поврежденный сектор,  
пропуск нескольких секторов после ошибки чтения сэкономит время 
и уменьшит износ привода.
Если вы хотите лишь быстро проверить поврежденный носитель, то хорошо установить значение этого параметра
в 1024. Но имейте в виду, что все пропущенные секторы считаются поврежденными,
поэтому число сообщаемых ошибок становится выше и менее точным.<p>
<?php end_screen_shot(); ?>

<!--- do not change below --->

<?php
# end_page() adds the footer line and closes the HTML properly.

end_page();
?>
