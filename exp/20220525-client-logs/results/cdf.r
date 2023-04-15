library(plyr)
args = commandArgs(trailingOnly = TRUE)
df <- read.csv(file = args[1], header = TRUE, sep = ",")

title <-unlist(strsplit(args[1], '\\.'))[1]

pdf(args[2])

plot(ecdf(df$time), main=title, xlab="SubmitTime (ms)")

print(sum(df$time))








