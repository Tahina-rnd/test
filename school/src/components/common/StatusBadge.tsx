import { cn } from '@/lib/utils';

type StatusType = 'present' | 'absent' | 'late' | 'excused' | 'paid' | 'partial' | 'unpaid' | 'active' | 'inactive';

interface StatusBadgeProps {
  status: StatusType;
  className?: string;
}

const statusStyles: Record<StatusType, string> = {
  present: 'bg-success/10 text-success border-success/20',
  absent: 'bg-destructive/10 text-destructive border-destructive/20',
  late: 'bg-warning/10 text-warning border-warning/20',
  excused: 'bg-muted text-muted-foreground border-muted',
  paid: 'bg-success/10 text-success border-success/20',
  partial: 'bg-warning/10 text-warning border-warning/20',
  unpaid: 'bg-destructive/10 text-destructive border-destructive/20',
  active: 'bg-success/10 text-success border-success/20',
  inactive: 'bg-muted text-muted-foreground border-muted',
};

const statusLabels: Record<StatusType, string> = {
  present: 'Present',
  absent: 'Absent',
  late: 'Late',
  excused: 'Excused',
  paid: 'Paid',
  partial: 'Partial',
  unpaid: 'Unpaid',
  active: 'Active',
  inactive: 'Inactive',
};

export function StatusBadge({ status, className }: StatusBadgeProps) {
  return (
    <span className={cn(
      "inline-flex items-center px-2.5 py-0.5 rounded-full text-xs font-medium border",
      statusStyles[status],
      className
    )}>
      {statusLabels[status]}
    </span>
  );
}
