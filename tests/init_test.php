<?php
	echo clips_init();
	echo clips_load(dirname(__FILE__).'/test.rules');
	echo clips_exec();
	echo clips_close();
