import { useState } from 'react';
import { Plus, Search, Edit, Trash2 } from 'lucide-react';
import { DashboardLayout } from '@/components/layout/DashboardLayout';
import { PageHeader } from '@/components/common/PageHeader';
import { DataTable } from '@/components/common/DataTable';
import { Button } from '@/components/ui/button';
import { Input } from '@/components/ui/input';
import {
  Dialog,
  DialogContent,
  DialogHeader,
  DialogTitle,
  DialogFooter,
} from '@/components/ui/dialog';
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from '@/components/ui/select';
import { Label } from '@/components/ui/label';
import { useToast } from '@/hooks/use-toast';
import { Card, CardContent, CardHeader, CardTitle } from '@/components/ui/card';

interface Grade {
  id: string;
  studentName: string;
  subject: string;
  score: number;
  maxScore: number;
  term: string;
  date: string;
}

// Mock data
const initialGrades: Grade[] = [
  { id: '1', studentName: 'Emma Johnson', subject: 'Mathematics', score: 85, maxScore: 100, term: 'Term 1', date: '2024-01-15' },
  { id: '2', studentName: 'Emma Johnson', subject: 'English', score: 92, maxScore: 100, term: 'Term 1', date: '2024-01-15' },
  { id: '3', studentName: 'Liam Smith', subject: 'Mathematics', score: 78, maxScore: 100, term: 'Term 1', date: '2024-01-15' },
  { id: '4', studentName: 'Liam Smith', subject: 'Science', score: 88, maxScore: 100, term: 'Term 1', date: '2024-01-15' },
  { id: '5', studentName: 'Olivia Brown', subject: 'History', score: 95, maxScore: 100, term: 'Term 1', date: '2024-01-15' },
  { id: '6', studentName: 'Olivia Brown', subject: 'Mathematics', score: 89, maxScore: 100, term: 'Term 1', date: '2024-01-15' },
  { id: '7', studentName: 'Noah Williams', subject: 'English', score: 72, maxScore: 100, term: 'Term 1', date: '2024-01-15' },
  { id: '8', studentName: 'Ava Davis', subject: 'Science', score: 91, maxScore: 100, term: 'Term 1', date: '2024-01-15' },
];

const classes = ['8-A', '8-B', '8-C', '9-A', '9-B', '10-A', '10-B', '11-A', '11-B', '12-A', '12-B'];
const subjects = ['Mathematics', 'English', 'Science', 'History', 'Geography', 'Physics', 'Chemistry', 'Biology'];
const terms = ['Term 1', 'Term 2', 'Term 3'];
const students = ['Emma Johnson', 'Liam Smith', 'Olivia Brown', 'Noah Williams', 'Ava Davis', 'Sophia Garcia', 'Mason Martinez'];

export default function Grades() {
  const [grades, setGrades] = useState<Grade[]>(initialGrades);
  const [classFilter, setClassFilter] = useState<string>('all');
  const [subjectFilter, setSubjectFilter] = useState<string>('all');
  const [searchQuery, setSearchQuery] = useState('');
  const [isModalOpen, setIsModalOpen] = useState(false);
  const [editingGrade, setEditingGrade] = useState<Grade | null>(null);
  const [formData, setFormData] = useState({
    studentName: '',
    subject: '',
    score: '',
    maxScore: '100',
    term: '',
    date: new Date().toISOString().split('T')[0],
  });
  const { toast } = useToast();

  const filteredGrades = grades.filter((grade) => {
    const matchesSearch = grade.studentName.toLowerCase().includes(searchQuery.toLowerCase());
    const matchesSubject = subjectFilter === 'all' || grade.subject === subjectFilter;
    return matchesSearch && matchesSubject;
  });

  // Calculate class average
  const classAverage = filteredGrades.length > 0
    ? (filteredGrades.reduce((acc, g) => acc + (g.score / g.maxScore) * 100, 0) / filteredGrades.length).toFixed(1)
    : '0';

  const handleOpenModal = (grade?: Grade) => {
    if (grade) {
      setEditingGrade(grade);
      setFormData({
        studentName: grade.studentName,
        subject: grade.subject,
        score: grade.score.toString(),
        maxScore: grade.maxScore.toString(),
        term: grade.term,
        date: grade.date,
      });
    } else {
      setEditingGrade(null);
      setFormData({
        studentName: '',
        subject: '',
        score: '',
        maxScore: '100',
        term: '',
        date: new Date().toISOString().split('T')[0],
      });
    }
    setIsModalOpen(true);
  };

  const handleSave = () => {
    if (!formData.studentName || !formData.subject || !formData.score || !formData.term) {
      toast({
        title: 'Error',
        description: 'Please fill in all required fields',
        variant: 'destructive',
      });
      return;
    }

    if (editingGrade) {
      setGrades(grades.map((g) =>
        g.id === editingGrade.id
          ? { ...g, ...formData, score: Number(formData.score), maxScore: Number(formData.maxScore) }
          : g
      ));
      toast({ title: 'Success', description: 'Grade updated successfully' });
    } else {
      const newGrade: Grade = {
        id: Date.now().toString(),
        ...formData,
        score: Number(formData.score),
        maxScore: Number(formData.maxScore),
      };
      setGrades([...grades, newGrade]);
      toast({ title: 'Success', description: 'Grade added successfully' });
    }
    setIsModalOpen(false);
  };

  const handleDelete = (id: string) => {
    setGrades(grades.filter((g) => g.id !== id));
    toast({ title: 'Success', description: 'Grade removed successfully' });
  };

  const getGradeColor = (percentage: number) => {
    if (percentage >= 90) return 'text-success';
    if (percentage >= 70) return 'text-primary';
    if (percentage >= 50) return 'text-warning';
    return 'text-destructive';
  };

  const columns = [
    { key: 'studentName', header: 'Student' },
    { key: 'subject', header: 'Subject' },
    {
      key: 'score',
      header: 'Score',
      render: (grade: Grade) => {
        const percentage = (grade.score / grade.maxScore) * 100;
        return (
          <span className={getGradeColor(percentage)}>
            {grade.score}/{grade.maxScore} ({percentage.toFixed(0)}%)
          </span>
        );
      },
    },
    { key: 'term', header: 'Term' },
    { key: 'date', header: 'Date' },
    {
      key: 'actions',
      header: '',
      className: 'w-24',
      render: (grade: Grade) => (
        <div className="flex gap-2">
          <Button variant="ghost" size="icon" onClick={() => handleOpenModal(grade)}>
            <Edit className="w-4 h-4" />
          </Button>
          <Button 
            variant="ghost" 
            size="icon" 
            onClick={() => handleDelete(grade.id)}
            className="text-destructive hover:text-destructive"
          >
            <Trash2 className="w-4 h-4" />
          </Button>
        </div>
      ),
    },
  ];

  return (
    <DashboardLayout>
      <PageHeader title="Grades" subtitle="View and manage student grades">
        <Button onClick={() => handleOpenModal()}>
          <Plus className="w-4 h-4 mr-2" /> Add Grade
        </Button>
      </PageHeader>

      {/* Stats Card */}
      <Card className="mb-6">
        <CardHeader className="pb-2">
          <CardTitle className="text-base font-medium text-muted-foreground">Class Average</CardTitle>
        </CardHeader>
        <CardContent>
          <div className="text-3xl font-bold text-foreground">{classAverage}%</div>
          <p className="text-sm text-muted-foreground">{filteredGrades.length} grades recorded</p>
        </CardContent>
      </Card>

      {/* Filters */}
      <div className="flex flex-col sm:flex-row gap-4 mb-6">
        <div className="relative flex-1">
          <Search className="absolute left-3 top-1/2 -translate-y-1/2 w-4 h-4 text-muted-foreground" />
          <Input
            placeholder="Search by student name..."
            value={searchQuery}
            onChange={(e) => setSearchQuery(e.target.value)}
            className="pl-10"
          />
        </div>
        <Select value={classFilter} onValueChange={setClassFilter}>
          <SelectTrigger className="w-full sm:w-40">
            <SelectValue placeholder="All Classes" />
          </SelectTrigger>
          <SelectContent>
            <SelectItem value="all">All Classes</SelectItem>
            {classes.map((cls) => (
              <SelectItem key={cls} value={cls}>{cls}</SelectItem>
            ))}
          </SelectContent>
        </Select>
        <Select value={subjectFilter} onValueChange={setSubjectFilter}>
          <SelectTrigger className="w-full sm:w-40">
            <SelectValue placeholder="All Subjects" />
          </SelectTrigger>
          <SelectContent>
            <SelectItem value="all">All Subjects</SelectItem>
            {subjects.map((subj) => (
              <SelectItem key={subj} value={subj}>{subj}</SelectItem>
            ))}
          </SelectContent>
        </Select>
      </div>

      {/* Table */}
      <DataTable
        columns={columns}
        data={filteredGrades}
        keyExtractor={(grade) => grade.id}
        emptyMessage="No grades found"
      />

      {/* Add/Edit Modal */}
      <Dialog open={isModalOpen} onOpenChange={setIsModalOpen}>
        <DialogContent className="sm:max-w-lg">
          <DialogHeader>
            <DialogTitle>{editingGrade ? 'Edit Grade' : 'Add New Grade'}</DialogTitle>
          </DialogHeader>
          <div className="grid gap-4 py-4">
            <div className="space-y-2">
              <Label>Student *</Label>
              <Select value={formData.studentName} onValueChange={(v) => setFormData({ ...formData, studentName: v })}>
                <SelectTrigger>
                  <SelectValue placeholder="Select student" />
                </SelectTrigger>
                <SelectContent>
                  {students.map((student) => (
                    <SelectItem key={student} value={student}>{student}</SelectItem>
                  ))}
                </SelectContent>
              </Select>
            </div>
            <div className="grid grid-cols-2 gap-4">
              <div className="space-y-2">
                <Label>Subject *</Label>
                <Select value={formData.subject} onValueChange={(v) => setFormData({ ...formData, subject: v })}>
                  <SelectTrigger>
                    <SelectValue placeholder="Select subject" />
                  </SelectTrigger>
                  <SelectContent>
                    {subjects.map((subj) => (
                      <SelectItem key={subj} value={subj}>{subj}</SelectItem>
                    ))}
                  </SelectContent>
                </Select>
              </div>
              <div className="space-y-2">
                <Label>Term *</Label>
                <Select value={formData.term} onValueChange={(v) => setFormData({ ...formData, term: v })}>
                  <SelectTrigger>
                    <SelectValue placeholder="Select term" />
                  </SelectTrigger>
                  <SelectContent>
                    {terms.map((term) => (
                      <SelectItem key={term} value={term}>{term}</SelectItem>
                    ))}
                  </SelectContent>
                </Select>
              </div>
            </div>
            <div className="grid grid-cols-2 gap-4">
              <div className="space-y-2">
                <Label htmlFor="score">Score *</Label>
                <Input
                  id="score"
                  type="number"
                  value={formData.score}
                  onChange={(e) => setFormData({ ...formData, score: e.target.value })}
                />
              </div>
              <div className="space-y-2">
                <Label htmlFor="maxScore">Max Score</Label>
                <Input
                  id="maxScore"
                  type="number"
                  value={formData.maxScore}
                  onChange={(e) => setFormData({ ...formData, maxScore: e.target.value })}
                />
              </div>
            </div>
            <div className="space-y-2">
              <Label htmlFor="date">Date</Label>
              <Input
                id="date"
                type="date"
                value={formData.date}
                onChange={(e) => setFormData({ ...formData, date: e.target.value })}
              />
            </div>
          </div>
          <DialogFooter>
            <Button variant="outline" onClick={() => setIsModalOpen(false)}>
              Cancel
            </Button>
            <Button onClick={handleSave}>
              {editingGrade ? 'Save Changes' : 'Add Grade'}
            </Button>
          </DialogFooter>
        </DialogContent>
      </Dialog>
    </DashboardLayout>
  );
}
