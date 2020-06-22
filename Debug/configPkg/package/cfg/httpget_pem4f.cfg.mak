# invoke SourceDir generated makefile for httpget.pem4f
httpget.pem4f: .libraries,httpget.pem4f
.libraries,httpget.pem4f: package/cfg/httpget_pem4f.xdl
	$(MAKE) -f C:\Users\noa\workspace_v10\ESE_FML_feat_httpget_EK_TM4C1294XL_TI2/src/makefile.libs

clean::
	$(MAKE) -f C:\Users\noa\workspace_v10\ESE_FML_feat_httpget_EK_TM4C1294XL_TI2/src/makefile.libs clean

