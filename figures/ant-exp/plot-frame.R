library(ggplot2)
library(ggpubr)
library(ggpattern)
library(dplyr)

text_size <- 35
title_size <-40
point_size <-25
cap_size <-50



myTheme <-theme(axis.text.x = element_text(size=text_size,color="black", margin=margin(t=15)),
  axis.text.y = element_text(size=text_size, color="black"),
  axis.title.x=element_text(size=title_size, color="black", face="bold",margin = margin(t = 15, r = 0, b = 0, l = 0)),
  axis.title.y=element_text(size=title_size,color="black", face="bold")) + 
  theme(legend.text = element_text(size = text_size), 
                                  legend.title = element_text(size = title_size, face="bold"), 
                                  legend.key.size = unit(2, 'cm'),
                                  # legend.position = c(20, 15),
                                  legend.box="vertical",
        legend.margin = margin(b=1.5, unit='cm')) +
  theme(panel.grid.major.y = element_line(color="black", size=0.75,linetype=2),
        panel.background = element_rect(fill='transparent'),
        plot.background = element_rect(fill='transparent', color=NA), 
        axis.line.y = element_line(arrow = grid::arrow(angle=15,
                                                       ends = "last", type="closed"),size=2),
        axis.line.x = element_line(arrow = grid::arrow(angle=15,
                                                       ends = "last", type="closed"),size=2))


data <- read.csv(file = 'data.csv')
data$System <-factor(data$System, levels=c("HSDS", "TileDB", "ArrayMorph"))
data$Platform <- factor(data$Platform, levels = c("Azure", "GCS", "S3"))

data <- data[data$Selectivity=="0.1" & (data$Chunking=="frame" | data$System=="ArrayMorph"),]
print(data)
p1<-ggplot(data, aes(Platform, Time)) + 
  geom_bar_pattern(stat = "identity", 
                   pattern = c("none", "stripe", "stripe", "none", "stripe", "stripe", "stripe"),
                   pattern_angle = c(0, 45, 45, 0,45, 135,135),
                   pattern_density = 0.05,
                   pattern_spacing = 0.02,
                   pattern_fill = 'black',
                   color="black",
                   aes(fill = System), 
                   position=position_dodge2(preserve = "single",width=.5)) +
  geom_text(aes(Platform, Time ,label=Choosen, group = System), 
            position = position_dodge2(width = .9), vjust = -.15, size = 18)+
  scale_fill_manual(" ", 
                    labels = c("HSDS", "TileDB", "ArrayMorph"),
                    values = c("HSDS" = "black", "TileDB"="grey30", "ArrayMorph"="white")) +
  guides(fill = guide_legend(title="System",
                              override.aes = 
                               list(
                                 pattern = c("none", "stripe", "stripe"),
                                 pattern_spacing = .1,
                                 pattern_density = .05,
                                 pattern_angle = c(0, 45, 135)
                                 )
                             )) +
myTheme +
labs(caption="(a) Time") +
theme(plot.caption=element_text(hjust=0.5, margin=margin(40,0,0,0),size=cap_size, face="bold"))+
xlab('Platform') +
ylab('Time(s)') +
scale_y_continuous(limits=c(0,2000), breaks=seq(0,2000,500),expand=c(0,0))

p2<-ggplot(data, aes(Platform, Cost)) + 
  geom_bar_pattern(stat = "identity", 
                   pattern = c("none", "stripe", "stripe", "none", "stripe", "stripe", "stripe"),
                   pattern_angle = c(0, 45, 45, 0,45, 135,135),
                   pattern_density = 0.05,
                   pattern_spacing = 0.02,
                   pattern_fill = 'black',
                   color="black",
                   aes(fill = System), 
                   position=position_dodge2(preserve = "single",width=.5)) +
    geom_text(aes(Platform, Cost ,label=Choosen, group = System), 
            position = position_dodge2(width = .9), vjust = -.15, size = 18)+
  scale_fill_manual(" ", 
                    labels = c("HSDS", "TileDB",  "ArrayMorph"),
                    values = c("HSDS" = "black", "TileDB"="grey30", "ArrayMorph"="white")) +
  guides(fill = guide_legend(title="System",
                              override.aes = 
                               list(
                                 pattern = c("none", "stripe",  "stripe"),
                                 pattern_spacing = .1,
                                 pattern_density = .05,
                                 pattern_angle = c(0, 45, 135)
                                 )
                             )) +
myTheme +
labs(caption="(b) Cost") +
theme(plot.caption=element_text(hjust=0.5, margin=margin(40,0,0,0),size=cap_size, face="bold"))+
xlab('Platform') +
ylab('Cost($)') +
scale_y_continuous(limits=c(0,2), breaks=c(0, 0.5, 1, 1.5, 2),expand=c(0,0))

p3<-ggplot(data, aes(Platform, Transfer.Size)) + 
  geom_bar_pattern(stat = "identity", 
                   pattern = c("none", "stripe", "stripe", "none", "stripe", "stripe", "stripe"),
                   pattern_angle = c(0, 45, 45, 0,45, 135,135),
                   pattern_density = 0.05,
                   pattern_spacing = 0.02,
                   pattern_fill = 'black',
                   color="black",
                   aes(fill = System), 
                   position=position_dodge2(preserve = "single",width=.5)) +
    geom_text(aes(Platform, Transfer.Size ,label=Choosen, group = System), 
            position = position_dodge2(width = .9), vjust = -.15, size = 18)+
  scale_fill_manual(" ", 
                    labels = c("HSDS", "TileDB",  "ArrayMorph"),
                    values = c("HSDS" = "black", "TileDB"="grey30", "ArrayMorph"="white")) +
  guides(fill = guide_legend(title="System",
                              override.aes = 
                               list(
                                 pattern = c("none", "stripe",  "stripe"),
                                 pattern_spacing = .1,
                                 pattern_density = .05,
                                 pattern_angle = c(0, 45, 135)
                                 )
                             )) +
myTheme +
labs(caption="(c) Transferred Data Size") +
theme(plot.caption=element_text(hjust=0.5, margin=margin(40,0,0,0),size=cap_size, face="bold"))+
xlab('Platform') +
ylab('Transferred Data Size(GB)') +
scale_y_continuous(limits=c(0,20), breaks=c(0,5,10,15,20),expand=c(0,0))

p <- ggarrange(p1, NULL, p2, NULL,p3, nrow=1, common.legend = TRUE, widths = c(1,0.1,1,0.1,1))


ggsave(file="ant-frame.eps", plot=p, width=45, height=12, device="eps")



