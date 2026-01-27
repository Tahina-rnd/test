import { useState } from 'react';
import { Save, ChevronLeft, ChevronRight } from 'lucide-react';
import { DashboardLayout } from '@/components/layout/DashboardLayout';
import { PageHeader } from '@/components/common/PageHeader';
import { StatusBadge } from '@/components/common/StatusBadge';
import { Button } from '@/components/ui/button';
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from '@/components/ui/select';
import { useToast } from '@/hooks/use-toast';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';
import { cn } from '@/lib/utils';

interface AttendanceRecord {
  studentId: string;
  studentName: string;
  status: 'present' | 'absent' | 'late' | 'excused';
}

// Mock students
const classStudents = [
  { id: '1', name: 'Emma Johnson' },
  { id: '2', name: 'Liam Smith' },
  { id: '3', name: 'Olivia Brown' },
  { id: '4', name: 'Noah Williams' },
  { id: '5', name: 'Ava Davis' },
  { id: '6', name: 'Sophia Garcia' },
  { id: '7', name: 'Mason Martinez' },
  { id: '8', name: 'Isabella Lopez' },
  { id: '9', name: 'Lucas Wilson' },
  { id: '10', name: 'Mia Anderson' },
];

const classes = ['8-A', '8-B', '8-C', '9-A', '9-B', '10-A', '10-B', '11-A', '11-B', '12-A', '12-B'];

const statusOptions: { value: AttendanceRecord['status']; label: string }[] = [
  { value: 'present', label: 'Present' },
  { value: 'absent', label: 'Absent' },
  { value: 'late', label: 'Late' },
  { value: 'excused', label: 'Excused' },
];

export default function Attendance() {
  const [selectedClass, setSelectedClass] = useState<string>('10-A');
  const [selectedDate, setSelectedDate] = useState<string>(new Date().toISOString().split('T')[0]);
  const [attendance, setAttendance] = useState<AttendanceRecord[]>(
    classStudents.map((s) => ({ 
      studentId: s.id, 
      studentName: s.name, 
      status: 'present' as const 
    }))
  );
  const { toast } = useToast();

  const handleStatusChange = (studentId: string, status: AttendanceRecord['status']) => {
    setAttendance((prev) =>
      prev.map((record) =>
        record.studentId === studentId ? { ...record, status } : record
      )
    );
  };

  const handleSave = () => {
    // In production, this would call the API
    toast({ title: 'Success', description: 'Attendance saved successfully' });
  };

  const navigateDate = (direction: 'prev' | 'next') => {
    const date = new Date(selectedDate);
    date.setDate(date.getDate() + (direction === 'prev' ? -1 : 1));
    setSelectedDate(date.toISOString().split('T')[0]);
  };

  const stats = {
    present: attendance.filter((a) => a.status === 'present').length,
    absent: attendance.filter((a) => a.status === 'absent').length,
    late: attendance.filter((a) => a.status === 'late').length,
    excused: attendance.filter((a) => a.status === 'excused').length,
  };

  const attendanceRate = ((stats.present + stats.late) / attendance.length * 100).toFixed(0);

  return (
    <DashboardLayout>
      <PageHeader title="Attendance" subtitle="Mark and view student attendance">
        <Button onClick={handleSave}>
          <Save className="w-4 h-4 mr-2" /> Save Attendance
        </Button>
      </PageHeader>

      {/* Controls */}
      <div className="flex flex-col sm:flex-row gap-4 mb-6">
        <Select value={selectedClass} onValueChange={setSelectedClass}>
          <SelectTrigger className="w-full sm:w-40">
            <SelectValue placeholder="Select Class" />
          </SelectTrigger>
          <SelectContent>
            {classes.map((cls) => (
              <SelectItem key={cls} value={cls}>{cls}</SelectItem>
            ))}
          </SelectContent>
        </Select>

        <div className="flex items-center gap-2">
          <Button variant="outline" size="icon" onClick={() => navigateDate('prev')}>
            <ChevronLeft className="w-4 h-4" />
          </Button>
          <input
            type="date"
            value={selectedDate}
            onChange={(e) => setSelectedDate(e.target.value)}
            className="px-3 py-2 border border-input rounded-md bg-background text-foreground"
          />
          <Button variant="outline" size="icon" onClick={() => navigateDate('next')}>
            <ChevronRight className="w-4 h-4" />
          </Button>
        </div>
      </div>

      {/* Stats */}
      <div className="grid grid-cols-2 sm:grid-cols-5 gap-4 mb-6">
        <Card>
          <CardHeader className="pb-2">
            <CardTitle className="text-sm font-medium text-muted-foreground">Attendance Rate</CardTitle>
          </CardHeader>
          <CardContent>
            <div className="text-2xl font-bold text-primary">{attendanceRate}%</div>
          </CardContent>
        </Card>
        <Card>
          <CardHeader className="pb-2">
            <CardTitle className="text-sm font-medium text-muted-foreground">Present</CardTitle>
          </CardHeader>
          <CardContent>
            <div className="text-2xl font-bold text-success">{stats.present}</div>
          </CardContent>
        </Card>
        <Card>
          <CardHeader className="pb-2">
            <CardTitle className="text-sm font-medium text-muted-foreground">Absent</CardTitle>
          </CardHeader>
          <CardContent>
            <div className="text-2xl font-bold text-destructive">{stats.absent}</div>
          </CardContent>
        </Card>
        <Card>
          <CardHeader className="pb-2">
            <CardTitle className="text-sm font-medium text-muted-foreground">Late</CardTitle>
          </CardHeader>
          <CardContent>
            <div className="text-2xl font-bold text-warning">{stats.late}</div>
          </CardContent>
        </Card>
        <Card>
          <CardHeader className="pb-2">
            <CardTitle className="text-sm font-medium text-muted-foreground">Excused</CardTitle>
          </CardHeader>
          <CardContent>
            <div className="text-2xl font-bold text-muted-foreground">{stats.excused}</div>
          </CardContent>
        </Card>
      </div>

      {/* Attendance List */}
      <div className="table-container">
        <div className="divide-y divide-border">
          {attendance.map((record) => (
            <div
              key={record.studentId}
              className="flex flex-col sm:flex-row sm:items-center justify-between p-4 gap-4 hover:bg-muted/30 transition-colors"
            >
              <div className="flex items-center gap-4">
                <div className="w-10 h-10 rounded-full bg-primary/10 flex items-center justify-center">
                  <span className="text-sm font-medium text-primary">
                    {record.studentName.split(' ').map((n) => n[0]).join('')}
                  </span>
                </div>
                <div>
                  <div className="font-medium">{record.studentName}</div>
                  <div className="text-sm text-muted-foreground">Class {selectedClass}</div>
                </div>
              </div>
              
              <div className="flex items-center gap-2 sm:gap-4">
                <div className="flex gap-2">
                  {statusOptions.map((option) => (
                    <button
                      key={option.value}
                      onClick={() => handleStatusChange(record.studentId, option.value)}
                      className={cn(
                        "px-3 py-1.5 text-sm font-medium rounded-lg border transition-colors",
                        record.status === option.value
                          ? option.value === 'present' ? 'bg-success/10 text-success border-success/20' :
                            option.value === 'absent' ? 'bg-destructive/10 text-destructive border-destructive/20' :
                            option.value === 'late' ? 'bg-warning/10 text-warning border-warning/20' :
                            'bg-muted text-muted-foreground border-muted'
                          : 'bg-background text-muted-foreground border-border hover:bg-muted'
                      )}
                    >
                      {option.label}
                    </button>
                  ))}
                </div>
              </div>
            </div>
          ))}
        </div>
      </div>
    </DashboardLayout>
  );
}
