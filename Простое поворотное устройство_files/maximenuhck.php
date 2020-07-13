
/*-----------------------------------------------------------------------------------------------------------
	This theme is largely inspired by the Mega menu tutorial on net.tutsplus.com :
	http://net.tutsplus.com/tutorials/html-css-techniques/how-to-build-a-kick-butt-css3-mega-drop-down-menu/
	
	Ce theme est largement inspire du tutoriel de Mega menu sur net.tutsplus.com
	http://net.tutsplus.com/tutorials/html-css-techniques/how-to-build-a-kick-butt-css3-mega-drop-down-menu/
-------------------------------------------------------------------------------------------------------------*/

.clr {clear:both;}

/*---------------------------------------------
---	 	menu container						---
----------------------------------------------*/

/* menu */
div#maximenuck {
	font-size:14px; 
	line-height:12px;
	text-align:left;
}

/* container style */
div#maximenuck ul.maximenuck {
    overflow: visible !important;
	display: block !important;
	float: none !important;
	visibility: visible !important;
	list-style:none;
	/*width:960px; */
	margin:0 auto;
	height:33px;
	padding:0px 5px 0px 5px;
	-moz-border-radius: 5px;
	-webkit-border-radius: 5px;
	border-radius: 2px;
        background: #014464;
        background: -moz-linear-gradient(top,  #eeeeee 0%, #aaaaaa 80%);
        background: -webkit-gradient(linear, left top, left bottom, color-stop(0%,#eeeeee), color-stop(70%,#aaaaaa));
        background: -webkit-linear-gradient(top,  #eeeeee 0%,#0aaaaaa 80%);
        background: -o-linear-gradient(top,  #eeeeee 0%,#aaaaaa 80%);
        background: -ms-linear-gradient(top,  #eeeeee 0%,#aaaaaa 80%);
        background: linear-gradient(top,  #eeeeee 0%,#aaaaaa 80%);
	border: 1px solid #002232;
	-moz-box-shadow:inset 0px 0px 1px #edf9ff;
	-webkit-box-shadow:inset 0px 0px 1px #edf9ff;
	box-shadow:inset 0px 0px 1px #edf9ff;
}

/*---------------------------------------------
---	 	Root items - level 1				---
----------------------------------------------*/

div#maximenuck ul.maximenuck li.maximenuck.level1 {
    background : none;
    list-style : none;
    border: 1px solid transparent;
	float:left;
	display:block;
	text-align:center;
	padding: 4px 4px 2px 4px;
	margin-right:25px !important;
	margin-top:2px !important;
}

div#maximenuck ul.maximenuck li.maximenuck.level1:hover,
div#maximenuck ul.maximenuck li.maximenuck.level1.active {
	border: 1px solid #777777;
	background: #F4F4F4;
	background: -moz-linear-gradient(top, #F4F4F4, #EEEEEE);
	background: -webkit-gradient(linear, 0% 0%, 0% 100%, from(#F4F4F4), to(#EEEEEE));
	-moz-border-radius: 3px;
	-webkit-border-radius: 3px;
	border-radius: 3px;
}

div#maximenuck ul.maximenuck li.maximenuck.level1 > a,
div#maximenuck ul.maximenuck li.maximenuck.level1 > span.separator {
	font-size:14px; 
	color: #bbbbbb;
	display:block;
	text-decoration:none;
	text-shadow: 1px 1px 1px #000;
	min-height : 22px;
    outline : none;
    background : none;
    border : none;
    padding : 5;
    white-space: normal;
}

/* parent item on mouseover (if subemnus exists) */
div#maximenuck ul.maximenuck li.maximenuck.level1.parent:hover,
div#maximenuck ul.maximenuck li.maximenuck.level1.parent:hover {
	-moz-border-radius: 3px 3px 0px 0px;
	-webkit-border-radius: 3px 3px 0px 0px;
	border-radius: 3px 3px 0px 0px;
}

/* item color on mouseover */
div#maximenuck ul.maximenuck li.maximenuck.level1:hover > a span.titreck,
div#maximenuck ul.maximenuck li.maximenuck.level1.active > a span.titreck,
div#maximenuck ul.maximenuck li.maximenuck.level1:hover > span.separator,
div#maximenuck ul.maximenuck li.maximenuck.level1.active > span.separator {
    color : #161616;
	text-shadow: 1px 1px 1px #ffffff;
}

/* arrow image for parent item */
div#maximenuck ul.maximenuck li.level1.parent > a,
div#maximenuck ul.maximenuck li.level1.parent > span.separator {
	padding-right:18px;
	background:url("../images/drop.gif") no-repeat right 8px !important;
}

div#maximenuck ul.maximenuck li.level1.parent:hover > a,
div#maximenuck ul.maximenuck li.level1.parent:hover > span.separator {
	background:url("../images/drop.gif") no-repeat right 8px !important;
}

/* arrow image for submenu parent item */
div#maximenuck ul.maximenuck li.level1.parent li.parent > a,
div#maximenuck ul.maximenuck li.level1.parent li.parent > span.separator,
div#maximenuck ul.maximenuck li.maximenuck ul.maximenuck2 li.parent:hover > a,
div#maximenuck ul.maximenuck li.maximenuck ul.maximenuck2 li.parent.active > a {
	padding-right:18px;
	background:url("../images/drop-right.gif") no-repeat right 8px !important;
}

/* styles for right position */
div#maximenuck ul.maximenuck li.menu_right {
	float:right !important;
	margin-right:0px !important;
}

div#maximenuck ul.maximenuck li.align_right div.floatck, 
div#maximenuck ul.maximenuck li div.floatck.fixRight {
	left:auto;
	right:-1px;
	top:auto;
	-moz-border-radius: 3px 0px 3px 3px;
    -webkit-border-radius: 3px 0px 3px 3px;
    border-radius: 3px 0px 3px 3px;
}


/* arrow image for submenu parent item to open left */
div#maximenuck ul.maximenuck li.level1.parent div.floatck.fixRight li.parent > a,
div#maximenuck ul.maximenuck li.level1.parent div.floatck.fixRight li.parent > span.separator,
div#maximenuck ul.maximenuck li.level1.parent.menu_right li.parent > a,
div#maximenuck ul.maximenuck li.level1.parent.menu_right li.parent > span.separator {
	padding-left:10px;
	background:url("../images/drop-left.gif") no-repeat left 8px !important;
}

/* margin for right elements that rolls to the left */
div#maximenuck ul.maximenuck li.maximenuck div.floatck div.floatck.fixRight,
div#maximenuck ul.maximenuck li.level1.parent.menu_right div.floatck div.floatck  {
    margin-right : 180px;
}

div#maximenuck ul.maximenuck li div.floatck.fixRight{
	-moz-border-radius: 3px 3px 3px 3px;
    -webkit-border-radius: 3px 0px 3px 3px;
    border-radius: 3px 0px 3px 3px;
}


/*---------------------------------------------
---	 	Sublevel items - level 2 to n		---
----------------------------------------------*/

div#maximenuck ul.maximenuck li div.floatck ul.maximenuck2 {
    background : transparent;
    margin : 0;
    padding : 0;
    border : none;
    width : 100%; /* important for Chrome and Safari compatibility */
    position: static;
}

div#maximenuck ul.maximenuck li ul.maximenuck2 li.maximenuck {
	font-size:12px;
	position:relative;
	text-shadow: 1px 1px 1px #ffffff;
	padding: 5px 0px !important;
	margin: 0px 0px 4px 0px !important;
	float:none !important;
	text-align:left;
	background : none;
    list-style : none;
	display: block !important;
}

div#maximenuck ul.maximenuck li ul.maximenuck2 li.maximenuck:hover {
	background: transparent;
}

/* all links styles */
div#maximenuck ul.maximenuck li.maximenuck a,
div#maximenuck ul.maximenuck li.maximenuck span.separator {
	font-size:14px; 
	font-weight : normal;
	color: #a1a1a1;
	display:block;
	text-decoration:none;
	text-transform : none;
	/*text-shadow: 1px 1px 1px #000;*/
    out
line : none;
    background : none;
    border : none;
    padding : 0 5px;
    white-space: normal;
}

/* submenu link */
div#maximenuck ul.maximenuck li.maximenuck ul.maximenuck2 li a {
	color:#015b86;
	text-shadow: 1px 1px 1px #ffffff;
}

div#maximenuck ul.maximenuck li.maximenuck ul.maximenuck2 a {
	font-size:12px;
	color:#161616;
	display: block;
}

div#maximenuck ul.maximenuck li.maximenuck ul.maximenuck2 li:hover > a,
div#maximenuck ul.maximenuck li.maximenuck ul.maximenuck2 li:hover > h2 a,
div#maximenuck ul.maximenuck li.maximenuck ul.maximenuck2 li:hover > h3 a,
div#maximenuck ul.maximenuck li.maximenuck ul.maximenuck2 li.active > a {
	color:#029feb !important;
	background: transparent !important;
}


/* link image style */
div#maximenuck ul.maximenuck li.maximenuck > a img {
    margin : 3px;
    border : none;
}

/* img style without link (in separator) */
div#maximenuck ul.maximenuck li.maximenuck img {
    border : none;
}

/* item title */
div#maximenuck span.titreck {
    /*text-transform : none;
    font-weight : normal;
    font-size : 10px;
    line-height : 10px;*/
    text-decoration : none;
    min-height : 10px;
    float : none !important;
    float : left;
}

/* item description */
div#maximenuck span.descck {
    display : block;
    text-transform : none;
    font-size : 10px;
    text-decoration : none;
    height : 12px;
    line-height : 12px;
    float : none !important;
    float : left;
}

/* submenus container */
div#maximenuck ul.maximenuck li div.floatck {
	width : 180px; /* default width */
	margin: 2px 0 0 -10px;
	text-align:left;
	padding:5px 5px 0 5px;
	border:1px solid #777777;
	border-top:none;
	background:#F4F4F4;
	background: -moz-linear-gradient(top, #EEEEEE, #BBBBBB);
	background: -webkit-gradient(linear, 0% 0%, 0% 100%, from(#EEEEEE), to(#BBBBBB));
	-moz-border-radius: 0px 3px 3px 3px;
	-webkit-border-radius: 0px 3px 3px 3px;
	border-radius: 0px 3px 3px 3px;
}

/*---------------------------------------------
---	 	Columns management					---
----------------------------------------------*/

/* child blocks position (from level2 to n) */
div#maximenuck ul.maximenuck li.maximenuck div.floatck div.floatck {
    margin : -10px 0 0 180px;
	-moz-border-radius: 3px;
	-webkit-border-radius: 3px;
	border-radius: 3px;
	border:1px solid #777777;
}

div#maximenuck ul.maximenuck li div.floatck div.maximenuck2 {
    width : 180px; /* default width */
	margin: 0;
	padding: 0;
}


/* h2 title */
div#maximenuck ul.maximenuck li.maximenuck ul.maximenuck2 h2 a,
div#maximenuck ul.maximenuck li.maximenuck ul.maximenuck2 h2 span.separator {
	font-size:10px;
	font-weight:100;
	letter-spacing:-1px;
	margin:7px 0 5px 0;
	padding-bottom:10px;
	border-bottom:1px solid #666666;
	line-height:10px;
	text-align:left;
}

/* h3 title */
div#maximenuck ul.maximenuck li.maximenuck ul.maximenuck2 h3 a,
div#maximenuck ul.maximenuck li.maximenuck ul.maximenuck2 h3 span.separator {
	font-size:12px;
	margin:7px 0 10px 0;
	padding-bottom:7px;
	border-bottom:1px solid #888888;
	line-height:10px;
	text-align:left;
}

/* paragraph */
div#maximenuck ul.maximenuck li ul.maximenuck2 li p {
	line-height:12px;
	margin:0 0 5px 0;
	font-size:12px;
	text-align:left;
}




/* image shadow with specific class */
div#maximenuck ul.maximenuck .imgshadow { /* Better style on light background */
	background:#FFFFFF !important;
	padding:4px;
	border:1px solid #777777;
	margin-top:5px;
	-moz-box-shadow:0px 0px 5px #666666;
	-webkit-box-shadow:0px 0px 5px #666666;
	box-shadow:0px 0px 5px #666666;
}

/* blackbox style */
div#maximenuck ul.maximenuck li ul.maximenuck2 li.blackbox {
	background-color:#333333 !important;
	color: #eeeeee;
	text-shadow: 1px 1px 1px #000;
	padding:4px 6px 4px 6px !important;
	margin: 0px 4px 4px 4px !important;
	-moz-border-radius: 5px;
    -webkit-border-radius: 5px;
    border-radius: 5px;
	-webkit-box-shadow:inset 0 0 3px #000000;
	-moz-box-shadow:inset 0 0 3px #000000;
	box-shadow:inset 0 0 3px #000000;
}

div#maximenuck ul.maximenuck li ul.maximenuck2 li.blackbox:hover {
	background-color:#333333 !important;
}

div#maximenuck ul.maximenuck li ul.maximenuck2 li.blackbox a {
	color: #fff;
	text-shadow: 1px 1px 1px #000;
	display: inline !important;
}

div#maximenuck ul.maximenuck li ul.maximenuck2 li.blackbox:hover > a {
	text-decoration: underline;
}

/* greybox style */
div#maximenuck ul.maximenuck li ul.maximenuck2 li.greybox {
	background:#f0f0f0 !important;
	border:1px solid #bbbbbb;
	padding: 4px 6px 4px 6px !important;
	margin: 0px 4px 4px 4px !important;
	-moz-border-radius: 3px;
    -webkit-border-radius: 3px;
    -khtml-border-radius: 3px;
    border-radius: 3px;
}

div#maximenuck ul.maximenuck li ul.maximenuck2 li.greybox:hover {
	background:#ffffff !important;
	border:1px solid #aaaaaa;
}


/*---------------------------------------------
---	 	Module in submenus					---
----------------------------------------------*/

/* module title */
div#maximenuck ul.maximenuck div.maximenuck_mod > div > h3 {
    width : 100%;
    font-weight : bold;
	color: #555;
	border-bottom: 1px solid #555;
	text-shadow: 1px 1px 1px #000;
	font-size: 10px;
}

div#maximenuck div.maximenuck_mod {
    width : 100%;
    padding : 0;
    white-space : normal;
}

div#maximenuck div.maximenuck_mod div.moduletable {
    border : none;
    background : none;
}

div#maximenuck div.maximenuck_mod  fieldset{
    width : 100%;
    padding : 0;
    margin : 0 auto;
    overflow : hidden;
    background : transparent;
    border : none;
}

div#maximenuck ul.maximenuck2 div.maximenuck_mod a {
    border : none;
    margin : 0;
    padding : 0;
    display : inline;
    background : transparent;
    font-weight : normal;
}

div#maximenuck ul.maximenuck2 div.maximenuck_mod a:hover {

}

div#maximenuck ul.maximenuck2 div.maximenuck_mod ul {
    margin : 0;
    padding : 0;
    width : 100%;
    background : none;
    border : none;
    text-align : left;
}

div#maximenuck ul.maximenuck2 div.maximenuck_mod li {
    margin : 0 0 0 5px;
    padding : 0;
    background : none;
    border : none;
    text-align : left;
    font-size : 11px;
    float : none;
    display : block;
    line-height : 12px;
    white-space : normal;
}

/* login module */
div#maximenuck ul.maximenuck2 div.maximenuck_mod #form-login ul {
    left : 0;
    margin : 0;
    padding : 0;
    width : 100%;
}

div#maximenuck ul.maximenuck2 div.maximenuck_mod #form-login ul li {
    margin : 2px 0;
    padding : 0 5px;
    height : 12px;
    background : transparent;
}



/*---------------------------------------------
---	 	Fancy styles (floating cursor)		---
----------------------------------------------*/

div#maximenuck .maxiFancybackground {
    list-style : none;
    padding: 0 !important;
    margin: 0 !important;
    border: none !important;
}

div#maximenuck .maxiFancybackground .maxiFancycenter {
    border-top: 1px solid #fff;
}



/*---------------------------------------------
---	 	Button to close on click			---
----------------------------------------------*/

div#maximenuck span.maxiclose {
    color: #fff;
}


