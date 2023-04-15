args = commandArgs(trailingOnly = TRUE)

df <- read.csv(file = args[1], header = TRUE, sep = ",")

s1 <- min(df$start)

df$start <- df$start - s1
df$finish <- df$finish - s1

e <- max(df$finish)
print(e)
pdf(args[2])
plot(NULL, xaxt="n", xlim=c(0, e), ylim=c(0, nrow(df)), xlab="Time (s)", ylab="request")
axis(side=1, at=c(seq(from=0, to=e, by=10)))
axis(side=2, at=c(seq(from=0, to=nrow(df), by=200)))
y <- 0

for (i in 1:nrow(df)) {
	start = df$start[i]
	end = df$finish[i]
	lines(x=c(start, end), y=c(y, y), col='red')
	y = y + 1
}

dev.off()