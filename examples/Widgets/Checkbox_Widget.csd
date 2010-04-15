<CsoundSynthesizer>
<CsOptions>
</CsOptions>
<CsInstruments>

sr = 44100
ksmps = 128
nchnls = 2
0dbfs = 1

instr 1
	khipass invalue "hipass"
	klowpass invalue "lowpass"
	kbandpass invalue "bandpass"
	asig noise 1, 0
	if (khipass == 1) then
		asig atone asig, 3000
		asig atone asig, 3000
		asig atone asig, 3000
	endif
	if (klowpass == 1) then
		asig tone asig, 8000
		asig tone asig, 8000
		asig tone asig, 8000
	endif
	dispfft asig*20, 0.25, 1024
	outs asig*0.1, asig*0.1
endin

instr 2 ; set checkbox values
	kstate init 0
	ktrig metro 1
	if (ktrig == 1) then
		kstate = (kstate == 0 ? 1 : 0)
	endif
	kenable invalue "enable"
	if (kenable == 1) then
		outvalue "checkbox", kstate
	endif
endin

</CsInstruments>
<CsScore>
i 1 0 1000
i 2 0 1000
</CsScore>
</CsoundSynthesizer>






<bsbPanel>
 <bgcolor mode="background">
  <r>138</r>
  <g>149</g>
  <b>156</b>
 </bgcolor>
 <bsbObject version="2" type="BSBLabel">
  <objectName/>
  <x>74</x>
  <y>6</y>
  <width>247</width>
  <height>44</height>
  <uuid>{ad8c5fad-d76a-425a-a10e-4b87d3142e8b}</uuid>
  <label>Checkbox Widget</label>
  <alignment>left</alignment>
  <font>DejaVu Sans</font>
  <fontsize>18</fontsize>
  <precision>3</precision>
  <color>
   <r>0</r>
   <g>0</g>
   <b>0</b>
  </color>
  <bgcolor mode="nobackground">
   <r>191</r>
   <g>204</g>
   <b>234</b>
  </bgcolor>
  <bordermode>noborder</bordermode>
  <borderradius>1</borderradius>
 </bsbObject>
 <bsbObject version="2" type="BSBLabel">
  <objectName/>
  <x>7</x>
  <y>45</y>
  <width>377</width>
  <height>56</height>
  <uuid>{35d4a9c6-98b9-4783-aed1-9ad640324b68}</uuid>
  <label>The checkbox widget is a simple widget which sends a value of 1 on its channel when its checked and a 0 when not. It's value can also be set using a channel</label>
  <alignment>left</alignment>
  <font>DejaVu Sans</font>
  <fontsize>7</fontsize>
  <precision>3</precision>
  <color>
   <r>0</r>
   <g>0</g>
   <b>0</b>
  </color>
  <bgcolor mode="background">
   <r>191</r>
   <g>204</g>
   <b>234</b>
  </bgcolor>
  <bordermode>noborder</bordermode>
  <borderradius>1</borderradius>
 </bsbObject>
 <bsbObject version="2" type="BSBLabel">
  <objectName/>
  <x>35</x>
  <y>111</y>
  <width>80</width>
  <height>25</height>
  <uuid>{c9d833b8-a125-42dc-bddd-fe01cedac125}</uuid>
  <label>Hi Pass filter</label>
  <alignment>left</alignment>
  <font>Helvetica</font>
  <fontsize>5</fontsize>
  <precision>3</precision>
  <color>
   <r>0</r>
   <g>0</g>
   <b>0</b>
  </color>
  <bgcolor mode="nobackground">
   <r>255</r>
   <g>255</g>
   <b>255</b>
  </bgcolor>
  <bordermode>noborder</bordermode>
  <borderradius>1</borderradius>
 </bsbObject>
 <bsbObject version="2" type="BSBLabel">
  <objectName/>
  <x>35</x>
  <y>138</y>
  <width>80</width>
  <height>25</height>
  <uuid>{660ba30b-432e-4ba9-aa47-1fc6de0c1e14}</uuid>
  <label>Low Pass filter</label>
  <alignment>left</alignment>
  <font>Helvetica</font>
  <fontsize>5</fontsize>
  <precision>3</precision>
  <color>
   <r>0</r>
   <g>0</g>
   <b>0</b>
  </color>
  <bgcolor mode="nobackground">
   <r>255</r>
   <g>255</g>
   <b>255</b>
  </bgcolor>
  <bordermode>noborder</bordermode>
  <borderradius>1</borderradius>
 </bsbObject>
 <bsbObject version="2" type="BSBGraph">
  <objectName/>
  <x>125</x>
  <y>106</y>
  <width>258</width>
  <height>263</height>
  <uuid>{d4ba5f84-006d-44f5-8ad1-4959acf47abb}</uuid>
  <value>0</value>
  <zoomx>1.00000000</zoomx>
  <zoomy>1.00000000</zoomy>
  <dispx>1.00000000</dispx>
  <dispy>1.00000000</dispy>
  <modex>lin</modex>
  <modey>lin</modey>
  <all>true</all>
 </bsbObject>
 <bsbObject version="2" type="BSBLabel">
  <objectName/>
  <x>11</x>
  <y>381</y>
  <width>372</width>
  <height>96</height>
  <uuid>{95f875a7-941b-45b5-b4f9-d353d1f2e483}</uuid>
  <label>Setting the value of a checkbox</label>
  <alignment>center</alignment>
  <font>DejaVu Sans</font>
  <fontsize>10</fontsize>
  <precision>3</precision>
  <color>
   <r>0</r>
   <g>0</g>
   <b>0</b>
  </color>
  <bgcolor mode="nobackground">
   <r>255</r>
   <g>255</g>
   <b>255</b>
  </bgcolor>
  <bordermode>border</bordermode>
  <borderradius>1</borderradius>
 </bsbObject>
 <bsbObject version="2" type="BSBCheckBox">
  <objectName>checkbox</objectName>
  <x>18</x>
  <y>441</y>
  <width>22</width>
  <height>20</height>
  <uuid>{4784a930-dd88-408c-b5eb-9c17c28e0b5d}</uuid>
  <selected>false</selected>
  <label/>
  <value>1.00000000</value>
  <randomizable>false</randomizable>
 </bsbObject>
 <bsbObject version="2" type="BSBLabel">
  <objectName/>
  <x>37</x>
  <y>440</y>
  <width>313</width>
  <height>28</height>
  <uuid>{0cea6f98-f6a5-4bba-9a75-2fb84e817366}</uuid>
  <label>This Checkbox's value is set from Csound</label>
  <alignment>left</alignment>
  <font>DejaVu Sans</font>
  <fontsize>6</fontsize>
  <precision>3</precision>
  <color>
   <r>0</r>
   <g>0</g>
   <b>0</b>
  </color>
  <bgcolor mode="nobackground">
   <r>255</r>
   <g>255</g>
   <b>255</b>
  </bgcolor>
  <bordermode>noborder</bordermode>
  <borderradius>1</borderradius>
 </bsbObject>
 <bsbObject version="2" type="BSBCheckBox">
  <objectName>enable</objectName>
  <x>18</x>
  <y>410</y>
  <width>21</width>
  <height>22</height>
  <uuid>{f773da97-1a7b-4950-aa2b-146323c34897}</uuid>
  <selected>false</selected>
  <label/>
  <value>1.00000000</value>
  <randomizable>false</randomizable>
 </bsbObject>
 <bsbObject version="2" type="BSBLabel">
  <objectName/>
  <x>35</x>
  <y>410</y>
  <width>336</width>
  <height>27</height>
  <uuid>{6e3bc69c-c7b1-4f5e-a83c-73474a679066}</uuid>
  <label>Tick this checkbox to enable changing the one below</label>
  <alignment>left</alignment>
  <font>DejaVu Sans</font>
  <fontsize>6</fontsize>
  <precision>3</precision>
  <color>
   <r>0</r>
   <g>0</g>
   <b>0</b>
  </color>
  <bgcolor mode="nobackground">
   <r>255</r>
   <g>255</g>
   <b>255</b>
  </bgcolor>
  <bordermode>noborder</bordermode>
  <borderradius>1</borderradius>
 </bsbObject>
 <bsbObject version="2" type="BSBCheckBox">
  <objectName>hipass</objectName>
  <x>19</x>
  <y>112</y>
  <width>23</width>
  <height>20</height>
  <uuid>{95ffad56-9b0a-4f4d-9698-26fea6b90a73}</uuid>
  <selected>false</selected>
  <label/>
  <value>1.00000000</value>
  <randomizable>false</randomizable>
 </bsbObject>
 <bsbObject version="2" type="BSBCheckBox">
  <objectName>lowpass</objectName>
  <x>19</x>
  <y>139</y>
  <width>22</width>
  <height>19</height>
  <uuid>{b931c42f-6de4-4e79-add7-eb71793915ab}</uuid>
  <selected>false</selected>
  <label/>
  <value>1.00000000</value>
  <randomizable>false</randomizable>
 </bsbObject>
 <objectName/>
 <x>692</x>
 <y>166</y>
 <width>406</width>
 <height>520</height>
 <visible>true</visible>
</bsbPanel>

<bsbPresets>
</bsbPresets>


<MacGUI>
ioView background {35466, 38293, 40092}
ioText {74, 6} {247, 44} display 0.000000 0.00100 "" left "DejaVu Sans" 18 {0, 0, 0} {65280, 65280, 65280} nobackground noborder Checkbox Widget
ioText {7, 45} {377, 56} display 0.000000 0.00100 "" left "DejaVu Sans" 7 {0, 0, 0} {48896, 52224, 59904} nobackground noborder The checkbox widget is a simple widget which sends a value of 1 on its channel when its checked and a 0 when not. It's value can also be set using a channel
ioText {35, 111} {80, 25} display 0.000000 0.00100 "" left "Helvetica" 5 {0, 0, 0} {65280, 65280, 65280} nobackground noborder Hi Pass filter
ioText {35, 138} {80, 25} display 0.000000 0.00100 "" left "Helvetica" 5 {0, 0, 0} {65280, 65280, 65280} nobackground noborder Low Pass filter
ioGraph {125, 106} {258, 263} table 0.000000 1.000000 
ioText {11, 381} {372, 96} display 0.000000 0.00100 "" center "DejaVu Sans" 10 {0, 0, 0} {65280, 65280, 65280} nobackground noborder Setting the value of a checkbox
ioCheckbox {18, 441} {22, 20} off checkbox
ioText {37, 440} {313, 28} display 0.000000 0.00100 "" left "DejaVu Sans" 6 {0, 0, 0} {65280, 65280, 65280} nobackground noborder This Checkbox's value is set from Csound
ioCheckbox {18, 410} {21, 22} off enable
ioText {35, 410} {336, 27} display 0.000000 0.00100 "" left "DejaVu Sans" 6 {0, 0, 0} {65280, 65280, 65280} nobackground noborder Tick this checkbox to enable changing the one below
ioCheckbox {19, 112} {23, 20} off hipass
ioCheckbox {19, 139} {22, 19} off lowpass
</MacGUI>

<EventPanel name="" tempo="60.00000000" loop="8.00000000" x="0" y="0" width="596" height="322"> 
 
 
 
 
 
 
 
 
 </EventPanel>