anaVersion="CastorMuons_201506012"
anaType="CastorMuons"

cbSmartCommand="smartCopy"
cbSmartBlackList=""
cbWMS="https://wmscms.cern.ch:7443/glite_wms_wmproxy_server"
skimEfficiencyMethod="getSkimEff"

sam = {}

sam["data_MinimumBias"]={}
sam["data_MinimumBias"]["crabJobs"]=100
sam["data_MinimumBias"]["GT"]='GR_P_V54::All'
sam["data_MinimumBias"]["name"]='data_MinimumBias'
sam["data_MinimumBias"]["isData"]=True
sam["data_MinimumBias"]["numEvents"]=-1
sam["data_MinimumBias"]["pathSE"]='srm://dcache-se-cms.desy.de:8443/srm/managerv2?SFN=/pnfs/desy.de/cms/tier2/store/user/sbaur/MinimumBias/CastorMuons_201506012_data_MinimumBias/150612_092132/0000/'
sam["data_MinimumBias"]["pathTrees"]='/XXXTMFTTree/store/user/sbaur/MinimumBias/CastorMuons_201506012_data_MinimumBias/150612_092132/0000//'
sam["data_MinimumBias"]["json"]='MinBias_CastorMuonRuns_json.txt'
sam["data_MinimumBias"]["XS"]=-1
sam["data_MinimumBias"]["pathPAT"]='/XXXTMFPAT/store/user/sbaur/MinimumBias/CastorMuons_201506012_data_MinimumBias/150612_092132/0000//'
sam["data_MinimumBias"]["DS"]='/MinimumBias/Run2015A-v1/RAW'


def fixLocalPaths(sam):
        import os,imp
        if "SmallXAnaDefFile" not in os.environ:
            print "Please set SmallXAnaDefFile environment variable:"
            print "export SmallXAnaDefFile=FullPathToFile"
            raise Exception("Whooops! SmallXAnaDefFile env var not defined")

        anaDefFile = os.environ["SmallXAnaDefFile"]
        mod_dir, filename = os.path.split(anaDefFile)
        mod, ext = os.path.splitext(filename)
        f, filename, desc = imp.find_module(mod, [mod_dir])
        mod = imp.load_module(mod, f, filename, desc)

        localBasePathPAT = mod.PATbasePATH
        localBasePathTrees = mod.TTreeBasePATH

        for s in sam:
            if "pathPAT" in sam[s]:
                sam[s]["pathPAT"] = sam[s]["pathPAT"].replace("XXXTMFPAT", localBasePathPAT)
            if "pathTrees" in sam[s]:
                sam[s]["pathTrees"] = sam[s]["pathTrees"].replace("XXXTMFTTree", localBasePathTrees)
            #print sam[s]["pathPAT"]
            #print sam[s]["pathTrees"]
        return sam
sam = fixLocalPaths(sam)
