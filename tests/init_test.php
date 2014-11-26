<?php
	echo clips_init();
	echo clips_load('/tmp/test.rules');
	echo clips_exec();
	echo clips_close();
