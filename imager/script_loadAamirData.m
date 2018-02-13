function label = script_loadAamirData(V)
% function f(V)
% function label = f('label')

if nargin==0, V=evalin('base','V'); end
if ischar(V) && strcmp(V,'label')
    label = 'Load Aamir data';
    return
end

% Config file
if isfield(V.content.user,'fconfig')
    % already a config file, the script will scan for new data instead of
    % re-loading everything
    fconfig = V.content.user.fconfig;
    dofull = false;
    info = V.content.user.info;
else
    fconfig = fn_getfile('config*.txt','Select config file');
    %fconfig = 'C:\Users\THomas\Desktop\TestExp008\config_file_TestExp008.txt';
    dofull = true;
    info = struct;
end

% Folders
folders = fn_readtext(fconfig);
folders(1) = [];
basefolder = fn_fileparts(fconfig,'dir');
folders = fn_strrep(folders,fn_fileparts(folders{1},'dir'),basefolder);
ok = false(1,length(folders));
for i=1:length(folders)
    ok(i) = exist(folders{i},'dir');
end
folders = folders(ok);

% Create folder for saving binned data
folderbin = fullfile(basefolder,'BinnedData');
if ~exist(folderbin,'dir'), mkdir(folderbin), end

% Ask for pixel size
if dofull
    s = struct('pixsize',5.6,'xbin',4, 'nframes',400);
    spec = struct('pixsize',{'double' 'Pixel size (um)'},'xbin',{'stepper 1 1 Inf' 'Spatial binning factor'},...
        'nframes',{'double','Total frames per trial'});
    s = fn_structedit(s,spec);
    xbin = s.xbin;
    pixsize = s.pixsize * xbin;
    info.xbin = xbin;
    info.pixsize = pixsize;
    nframes = s.nframes; 
    info.nframes = nframes; 
else
    xbin = info.xbin;
    pixsize = info.pixsize;
    nframes = info.nframes; 
    currentT = V.content.trials;
    currentTfiles = fn_strrep({currentT.file},'/',filesep,'\',filesep);
end

% Read data
ncond = length(folders);
for kcond = 1:ncond
    % folder
    folderk = folders{kcond};
    cd(folderk);
    sessionfolders = dir;
    sessionfolders(1:2) = [];
    sessionfolders(~[sessionfolders.isdir]) = [];
    sessionfolders = {sessionfolders.name};
    nSessions = length(sessionfolders);
    T = cell(1,nSessions);
    condnames = cell(1,nSessions);
    % sessions
    for ksession = 1:nSessions
        foldersess = fullfile(folderk,sessionfolders(ksession));
        % Read frame interval
        if dofull
            fgui = fullfile(strjoin(foldersess),'Trial_Parameters.txt');
            gui = fn_readtext(fgui);
            for i=1:length(gui)
                tokens = regexp(gui{i},'High phase duration[^\d]*(\d+)','tokens');
                if ~isempty(tokens)
                    highphase = str2double(tokens{1}{1});
                end
                tokens = regexp(gui{i},'Low phase duration[^\d]*(\d+)','tokens');
                if ~isempty(tokens), lowphase = str2double(tokens{1}{1}); end
            end
            dt = (highphase+lowphase)/1e3;
            info.dt = dt;
        else
            dt = info.dt;
        end
        condnames{ksession} = [fn_fileparts(folderk,'base') '-' fn_fileparts(strjoin(foldersess),'base')];
        cd(strjoin(foldersess));
        trialfolders = dir;
        trialfolders(1:2) = [];
        trialfolders(~[trialfolders.isdir]) = [];
        trialfolders = {trialfolders.name};
        ntrial = length(trialfolders);
        % data
        Tk = cell(1,ntrial);
        datek = zeros(1,ntrial);
        fn_progress(condnames{ksession},ntrial)
        for ktrial = 1:ntrial
            %fprintf('%s: trial %i/%i\n',condnames{kcond},ktrial,ntrial)
            fn_progress(ktrial)
            % check whether some binned data has been saved already for this
            % trial
            fsave = fullfile(folderbin,sprintf('%s-%s-bin%i.mat',condnames{ksession},trialfolders{ktrial},xbin));
            if dofull || ~ismember(fsave,currentTfiles)
                cd(strjoin(fullfile(foldersess,trialfolders{ktrial})));
                d = dir('*.tiff');
                while(length(d) ~= nframes)
                    d = dir('*.tiff');
                end
                [~, ord] = sort([d.datenum]);
                fimg = {d(ord).name};
                tiffinfo = imfinfo(fimg{1});
                datek(ktrial) = datenum(tiffinfo.FileModDate);
                if exist(fsave,'file') && fn_dialog_questandmem('Existing binned data has been detected. Use it?')
                    % create tps_trial object by using binned data from existing file
                    Tk{ktrial} = tps_trial(fsave,pixsize,dt);
                    Tk{ktrial}.user.date = datek(ktrial);
                else
                    % read the raw data
                    datak = fn_bin(imread(fimg{1})',xbin);
                    datak(1,1,length(fimg)) = 0;
                    for kframe = 2:length(fimg)
                        datak(:,:,kframe) = fn_bin(imread(fimg{kframe})',xbin);
                    end
                    % creat tps trial object
                    Tk{ktrial} = tps_trial(datak,pixsize,dt);
                    Tk{ktrial}.user.date = datek(ktrial);
                    % save the binned data
                    Tk{ktrial}.savedata(fsave)
                end
            else
                Tk{ktrial} = currentT(strcmp(fsave,currentTfiles));
                datek(ktrial) = Tk{ktrial}.user.date;
            end
        end
        [datek, ord] = sort(datek); 
        T{ksession} = [Tk{ord}];
        
        % set some header info
        T{ksession}.setstim(struct('name',condnames{ksession}));
    end;
    data{kcond,:} = [T{:}];
end
data = [data{:}];
% Load the file in the program
V.setdata(data)
% Store some info in OptImage
V.content.user.fconfig = fconfig;
V.content.user.info = info;

% T = V.content.trials;
% access(T)
% [T.sfrchannel] = deal(false);
% dbquit









