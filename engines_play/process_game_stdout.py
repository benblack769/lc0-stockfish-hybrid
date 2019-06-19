import sys

def get_value_by_name(line,name,num_items=1):
    if name not in line:
        raise RuntimeError("name not in line: "+line)
    start_idx = line.index(name)
    items = line[start_idx:].split()[1:1+num_items]
    return items if num_items > 1 else items[0]


class InfoCollector:
    def __init__(self):
        self.main_eval = None
        self.sf_eval = None
        self.pv = None
        self.allowed_move_dict = None
        self.rootmoves = None
        self.rootmovesdepth = None
        self.time_move = None
        self.depth = None
        self.seldepth = None
        self.bestprevmove = None
        self.bestsfprevmove = None

    def set_time(self,time):
        self.time_move = time

    def header_names(self):
        return [k for k,v in vars(self).items() if v is not None]

    def orderd_values(self,names):
        dict = vars(self)
        items = [dict[n] for n in names]
        return [item if item is not None else "NA" for item in items]

    def is_bestmove(self,uci_line):
        return "bestmove" in uci_line

    def set_info(self,uci_line):
        if "otherinfo" in uci_line:
            self.rootmovesdepth = (get_value_by_name(uci_line,"rootdepth"))
            self.rootmoves = (get_value_by_name(uci_line,"rootoptions"))
            self.allowed_move_dict = get_value_by_name(uci_line,"rootoptions")
        elif "short_info" in  uci_line:
            #nothing of use in this string
            pass
        elif "sf_info" in uci_line:
            self.sf_eval = " ".join(get_value_by_name(uci_line,"score",2))
            pass
        elif "info" in uci_line:
            if "score" in uci_line:
                self.main_eval = " ".join(get_value_by_name(uci_line,"score",2))
            if "pv " in uci_line:
                pv_idx = uci_line.index("pv ")
                pv_str = uci_line[pv_idx+2:].strip()
            #    self.pv = pv_str
            if "depth" in uci_line:
                self.depth = get_value_by_name(uci_line,"depth")
            if "time" in uci_line:
                self.time_move = get_value_by_name(uci_line,"time")
            if "seldepth" in uci_line:
                self.seldepth = get_value_by_name(uci_line,"seldepth")
        elif "bestmove" in uci_line:
            self.bestprevmove = get_value_by_name(uci_line,"bestmove")
        elif "best_ab_move" in uci_line:
            self.bestsfprevmove = get_value_by_name(uci_line,"best_ab_move")



def lines_to_csv(lines):
    return "\n".join(",".join(l) for l in lines)+"\n"

def file_to_info_csv(filename):
    with open(filename) as file:
        lines = file.readlines()
    collector = InfoCollector()
    csv_collectors = []
    for line in lines:
        if collector.is_bestmove(line):
            csv_collectors.append(collector)
            collector = InfoCollector()

        collector.set_info(line)

    if not csv_collectors:
        raise RuntimeError("file had zero 'bestmove' lines")
    csv_header = csv_collectors[1].header_names()
    csv_lines = [csv_header] + [col.orderd_values(csv_header) for col in csv_collectors]
    print(lines_to_csv(csv_lines))


def main():
    assert len(sys.argv) == 2, "needs 1 command line argument, the filename of the output to process"

    in_filename = sys.argv[1]
    file_to_info_csv(in_filename)

if __name__ == "__main__":
    main()
