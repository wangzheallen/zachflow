
function [] = extractOpticalFlow(index)
path1 = '/nfs/zhewang/Action/hmdb51_org_idt/';
path2 = '/nfs/zhewang/Action/hmdb51_flow_zach/';

folderlist = dir(path1);
foldername = {folderlist(:).name};
foldername = setdiff(foldername,{'.','..'});

for i = index
    if ~exist([path2,foldername{i}],'dir')
        mkdir([path2,foldername{i}]);
    end
    filelist = dir([path1,foldername{i},'/*.avi']);

    for j = 1:length(filelist)
        if ~exist([path2,foldername{i},'/',filelist(j).name(1:end-4)],'dir')
            mkdir([path2,foldername{i},'/',filelist(j).name(1:end-4)]);
        end 
        file1 = [path1,foldername{i},'/',filelist(j).name];
        file2 = [path2,foldername{i},'/',filelist(j).name(1:end-4),'/','flow_x'];
        file3 = [path2,foldername{i},'/',filelist(j).name(1:end-4),'/','flow_y'];
		file4 = [path2,foldername{i},'/',filelist(j).name(1:end-4),'/','flow_i'];
         cmd = sprintf('~/newFlow/newFlow -f %s -x %s -y %s -i %s -b 15',file1,file2,file3,file4);
         system(cmd);
	end
	i
end
end
